// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_lava_lamp.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::GradStop;

void FxLavaLamp::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    field_.clear();
    return;
  }
  FW_ = (W_ + DOWNSCALE - 1) / DOWNSCALE;
  FH_ = (H_ + DOWNSCALE - 1) / DOWNSCALE;
  field_.assign((size_t) FW_ * FH_, 0);
  fxutil::build_sin_lut_u8(sin8_);

  const float base_r = BLOB_R_FRAC * (float) std::min(W_, H_);
  for (int i = 0; i < NUM_BLOBS; ++i) {
    Blob &b = blobs_[i];
    b.x = frand(0.25f, 0.75f) * (float) W_;
    b.y = frand(0.3f, 0.7f) * (float) H_;
    b.vx = frand(1.5f, 3.5f) * (frand(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f);
    b.phase = frand(0.0f, 1.0f);
    b.phase_hz = frand(BUOY_HZ_MIN, BUOY_HZ_MAX);
    b.r = base_r * frand(0.8f, 1.15f);
  }

  // Palette-as-threshold: below the knee is the dark lamp fluid, above it
  // the hot blob body
  static const GradStop STOPS[] = {
      {0, 8, 2, 10},      // near black with a purple cast
      {80, 45, 8, 18},    // deep maroon halo
      {110, 120, 28, 18}, // knee: blob edge
      {170, 220, 90, 20}, // hot orange
      {255, 255, 200, 70} // molten yellow core
  };
  pal_.build(STOPS, 5);
}

void FxLavaLamp::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || field_.size() != (size_t) FW_ * FH_)
    return;

  // Blob motion: slow sine buoyancy plus a lateral drift with soft bounces
  int32_t bx[NUM_BLOBS], by[NUM_BLOBS], rq2[NUM_BLOBS];
  for (int i = 0; i < NUM_BLOBS; ++i) {
    Blob &b = blobs_[i];
    b.phase += b.phase_hz * dt;
    if (b.phase >= 1.0f)
      b.phase -= 1.0f;
    const float buoy = ((float) sin8_[(uint8_t) (b.phase * 256.0f)] - 128.0f) / 128.0f;  // -1..1
    b.y += buoy * 4.5f * dt;
    b.x += b.vx * dt;
    const float margin = b.r * 0.4f;
    if (b.x < margin) {
      b.x = margin;
      b.vx = std::abs(b.vx);
    } else if (b.x > (float) W_ - margin) {
      b.x = (float) W_ - margin;
      b.vx = -std::abs(b.vx);
    }
    b.y = fxutil::clampf(b.y, margin, (float) H_ - margin);

    bx[i] = (int32_t) b.x;
    by[i] = (int32_t) b.y;
    rq2[i] = (int32_t) (b.r * b.r) * FIELD_GAIN;
  }

  // Half-res metaball field: acc = sum(R^2*GAIN / (d^2 + 16))
  uint8_t *f = field_.data();
  for (int fy = 0; fy < FH_; ++fy) {
    const int32_t py = fy * DOWNSCALE;
    for (int fx = 0; fx < FW_; ++fx, ++f) {
      const int32_t px = fx * DOWNSCALE;
      int32_t acc = 0;
      for (int i = 0; i < NUM_BLOBS; ++i) {
        const int32_t dx = px - bx[i];
        const int32_t dy = py - by[i];
        acc += rq2[i] / (dx * dx + dy * dy + 16);
      }
      *f = (uint8_t) (acc > 255 ? 255 : acc);
    }
  }

  // Nearest-neighbor upscale through the palette
  for (int y = 0; y < v.h; ++y) {
    const uint8_t *src = &field_[(size_t) (y / DOWNSCALE) * FW_];
#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      const fxutil::NativePix c = pal_.e[src[x / DOWNSCALE]];
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    for (int x = 0; x < v.w; ++x)
      p[x] = pal_.e[src[x / DOWNSCALE]];
#endif
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
