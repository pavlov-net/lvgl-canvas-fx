// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_fireflies.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::clampf;
using fxutil::frand;
using fxutil::scale8;

void FxFireflies::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    flies_.clear();
    return;
  }
  fxutil::build_sin_lut_u8(sin8_);

  flies_.resize(NUM_FLIES);
  for (auto &f : flies_) {
    f.x = frand(2.0f, (float) W_ - 2.0f);
    f.y = frand(2.0f, (float) H_ - 2.0f);
    f.vx = frand(-VMAX, VMAX) * 0.5f;
    f.vy = frand(-VMAX, VMAX) * 0.5f;
    f.phase = frand(0.0f, 1.0f);
    f.pulse_hz = frand(PULSE_HZ_MIN, PULSE_HZ_MAX);
  }
}

void FxFireflies::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || flies_.empty())
    return;

  v.fade_to_black(TRAIL_FADE_OPA);

  for (auto &f : flies_) {
    // Slow random walk
    f.vx = clampf(f.vx + frand(-WANDER_ACCEL, WANDER_ACCEL) * dt, -VMAX, VMAX);
    f.vy = clampf(f.vy + frand(-WANDER_ACCEL, WANDER_ACCEL) * dt, -VMAX, VMAX);
    f.x += f.vx * dt;
    f.y += f.vy * dt;
    // Soft bounce at edges
    if (f.x < 1.0f) {
      f.x = 1.0f;
      f.vx = std::abs(f.vx);
    } else if (f.x > (float) W_ - 2.0f) {
      f.x = (float) W_ - 2.0f;
      f.vx = -std::abs(f.vx);
    }
    if (f.y < 1.0f) {
      f.y = 1.0f;
      f.vy = std::abs(f.vy);
    } else if (f.y > (float) H_ - 2.0f) {
      f.y = (float) H_ - 2.0f;
      f.vy = -std::abs(f.vy);
    }

    // Independent breathing; squared so each fly spends most of its cycle dark
    f.phase += f.pulse_hz * dt;
    if (f.phase >= 1.0f)
      f.phase -= 1.0f;
    const uint8_t s = sin8_[(uint8_t) (f.phase * 256.0f)];
    const uint8_t b = (uint8_t) (((uint16_t) s * s) >> 8);
    if (b < 6)
      continue;

    // 3x3 additive warm glow: bright core, dim edges, faint corners
    const int x = (int) f.x, y = (int) f.y;
    const uint8_t r = scale8(255, b), g = scale8(180, b), bl = scale8(50, b);
    v.put_px_add(x, y, r, g, bl);
    v.put_px_add(x - 1, y, r >> 1, g >> 1, bl >> 1);
    v.put_px_add(x + 1, y, r >> 1, g >> 1, bl >> 1);
    v.put_px_add(x, y - 1, r >> 1, g >> 1, bl >> 1);
    v.put_px_add(x, y + 1, r >> 1, g >> 1, bl >> 1);
    v.put_px_add(x - 1, y - 1, r / 5, g / 5, bl / 5);
    v.put_px_add(x + 1, y - 1, r / 5, g / 5, bl / 5);
    v.put_px_add(x - 1, y + 1, r / 5, g / 5, bl / 5);
    v.put_px_add(x + 1, y + 1, r / 5, g / 5, bl / 5);
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
