// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_warp.h"
#include <cmath>

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;

void FxWarp::respawn_(Star &s) {
  const float a = frand(0.0f, 2.0f * (float) M_PI);
  s.ux = std::cos(a);
  s.uy = std::sin(a);
  s.r = frand(0.5f, max_r_ * 0.25f);
}

void FxWarp::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    stars_.clear();
    return;
  }
  max_r_ = 0.5f * std::sqrt((float) (W_ * W_ + H_ * H_)) + 2.0f;
  stars_.resize(NUM_STARS);
  for (auto &s : stars_) {
    respawn_(s);
    s.r = frand(0.5f, max_r_);  // scatter initial depth
  }
}

void FxWarp::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || stars_.empty())
    return;

  v.fade_to_black(TRAIL_FADE_OPA);

  const float cx = (float) W_ * 0.5f, cy = (float) H_ * 0.5f;
  for (auto &s : stars_) {
    const float r0 = s.r;
    s.r += (s.r * GROW_RATE + BASE_SPEED) * dt;

    // Brightness ramps with distance travelled outward
    const float depth = fxutil::clampf(s.r / max_r_, 0.0f, 1.0f);
    const uint8_t b = (uint8_t) (40 + 215.0f * depth * depth);

    // Draw the streak from the previous radius to the current one
    const int steps = std::max(1, (int) (s.r - r0) + 1);
    for (int i = 0; i <= steps; ++i) {
      const float rr = r0 + (s.r - r0) * (float) i / (float) steps;
      const uint8_t bb = (uint8_t) ((b * (i + 1)) / (steps + 1));
      v.put_px_add((int) (cx + s.ux * rr), (int) (cy + s.uy * rr), bb, bb, fxutil::sat_add8(bb, bb >> 3));
    }

    if (s.r >= max_r_)
      respawn_(s);
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
