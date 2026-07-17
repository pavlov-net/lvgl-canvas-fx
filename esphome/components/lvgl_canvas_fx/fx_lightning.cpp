// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_lightning.h"
#include <cmath>

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::clampi;
using fxutil::frand;
using fxutil::irand;

void FxLightning::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0)
    return;

  // Dark storm sky: slightly lighter at the top (backlit clouds), near
  // black at the horizon
  base_r_.assign((size_t) H_, 0);
  base_g_.assign((size_t) H_, 0);
  base_b_.assign((size_t) H_, 0);
  for (int y = 0; y < H_; ++y) {
    const float t = (H_ > 1) ? (float) y / (float) (H_ - 1) : 0.0f;
    base_r_[y] = (uint8_t) (18.0f - 12.0f * t);
    base_g_[y] = (uint8_t) (22.0f - 14.0f * t);
    base_b_[y] = (uint8_t) (38.0f - 22.0f * t);
  }
  bolt_.clear();
  branch_.clear();
  bolt_ttl_ = 0;
  flash_ = 0;
  bolt_timer_ = frand(1.5f, BOLT_MAX_S * 0.5f);
}

void FxLightning::fire_bolt_() {
  bolt_.clear();
  branch_.clear();

  // Jagged random walk from a random top position to the bottom
  int x = irand(W_ / 5, W_ - 1 - W_ / 5);
  int y = 0;
  bolt_.push_back({(int16_t) x, (int16_t) y});
  while (y < H_ - 1) {
    y += irand(1, 2);
    x = clampi(x + irand(-2, 2), 0, W_ - 1);
    bolt_.push_back({(int16_t) x, (int16_t) clampi(y, 0, H_ - 1)});
  }

  // One shorter branch splitting off around the middle
  if (bolt_.size() > 6) {
    const Pt &src = bolt_[bolt_.size() / 2 + (size_t) irand(-2, 2)];
    int bx = src.x, by = src.y;
    const int dir = irand(0, 1) ? 1 : -1;
    const int len = (H_ - by) / 2;
    for (int i = 0; i < len && by < H_ - 1; ++i) {
      by += irand(1, 2);
      bx = clampi(bx + dir * irand(0, 2), 0, W_ - 1);
      branch_.push_back({(int16_t) bx, (int16_t) clampi(by, 0, H_ - 1)});
    }
  }

  bolt_ttl_ = BOLT_TTL_S;
  flash_ = 1.0f;
}

void FxLightning::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || base_r_.size() != (size_t) v.h)
    return;

  bolt_timer_ -= dt;
  if (bolt_timer_ <= 0.0f) {
    fire_bolt_();
    bolt_timer_ = frand(BOLT_MIN_S, BOLT_MAX_S);
    // Occasionally a quick double strike
    if (irand(0, 3) == 0)
      bolt_timer_ = frand(0.3f, 0.7f);
  }

  flash_ *= std::exp(-FLASH_DECAY * dt);
  if (flash_ < 0.01f)
    flash_ = 0;

  // Sky rows brightened by the flash (violet-tinged white)
  const int fl = (int) (flash_ * 255.0f);
  for (int y = 0; y < v.h; ++y) {
    const uint8_t r = (uint8_t) std::min(255, base_r_[y] + ((fl * 150) >> 8));
    const uint8_t g = (uint8_t) std::min(255, base_g_[y] + ((fl * 140) >> 8));
    const uint8_t b = (uint8_t) std::min(255, base_b_[y] + ((fl * 190) >> 8));
    const fxutil::NativePix c = fxutil::pack_rgb(r, g, b);
#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    std::fill(p, p + v.w, c);
#endif
  }

  // The bolt itself, flickering while alive
  if (bolt_ttl_ > 0.0f) {
    bolt_ttl_ -= dt;
    const float life = fxutil::clampf(bolt_ttl_ / BOLT_TTL_S, 0.0f, 1.0f);
    const uint8_t br = (uint8_t) ((160 + irand(0, 95)) * life);
    for (const auto &p : bolt_) {
      v.put_px_add(p.x, p.y, br, br, fxutil::sat_add8(br, br >> 2));
      v.put_px_add(p.x + 1, p.y, br >> 2, br >> 2, br >> 1);  // faint glow
      v.put_px_add(p.x - 1, p.y, br >> 2, br >> 2, br >> 1);
    }
    const uint8_t bb = br >> 1;
    for (const auto &p : branch_)
      v.put_px_add(p.x, p.y, bb, bb, fxutil::sat_add8(bb, bb >> 2));
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
