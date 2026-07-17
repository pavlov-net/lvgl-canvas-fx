// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_breathing.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;

void FxBreathing::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    base_.clear();
    return;
  }
  fxutil::build_sin_lut_u8(sin8_);

  // Radial falloff via squared distance (no sqrt): 255 at center, fading
  // toward the farthest corner.
  base_.assign((size_t) W_ * H_, 0);
  const int cx = W_ / 2, cy = H_ / 2;
  const int dxm = std::max(cx, W_ - 1 - cx);
  const int dym = std::max(cy, H_ - 1 - cy);
  const int d2max = std::max(1, dxm * dxm + dym * dym);
  for (int y = 0; y < H_; ++y) {
    const int dy2 = (y - cy) * (y - cy);
    for (int x = 0; x < W_; ++x) {
      const int d2 = (x - cx) * (x - cx) + dy2;
      base_[(size_t) y * W_ + x] = (uint8_t) (255 - (d2 * GRAD_DEPTH) / d2max);
    }
  }
}

void FxBreathing::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || base_.size() != (size_t) v.w * v.h)
    return;

  phase_ += dt / PERIOD_S;
  if (phase_ >= 1.0f)
    phase_ -= 1.0f;
  const uint8_t s = sin8_[(uint8_t) (phase_ * 256.0f)];
  const uint8_t br = (uint8_t) (MIN_BRIGHT + (((255 - MIN_BRIGHT) * s) >> 8));

  // Rebuild the 256-entry brightness LUT for this frame (cheap), then the
  // per-pixel work is a single lookup.
  for (int i = 0; i < 256; ++i) {
    const uint32_t k = (uint32_t) i * br;  // 0..65025
    lut_[i] = fxutil::pack_rgb((uint8_t) ((BASE_R * k) >> 16), (uint8_t) ((BASE_G * k) >> 16),
                               (uint8_t) ((BASE_B * k) >> 16));
  }

  for (int y = 0; y < v.h; ++y) {
    const uint8_t *src = &base_[(size_t) y * v.w];
#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      const fxutil::NativePix &c = lut_[src[x]];
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    for (int x = 0; x < v.w; ++x)
      p[x] = lut_[src[x]];
#endif
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
