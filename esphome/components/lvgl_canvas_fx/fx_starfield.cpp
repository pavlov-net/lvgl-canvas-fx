// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_starfield.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::irand;
using fxutil::scale8;

void FxStarfield::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    stars_.clear();
    return;
  }
  fxutil::build_sin_lut_u8(sin8_);

  const int n = std::max(4, W_ * H_ / STAR_DIV);
  stars_.resize((size_t) n);
  for (auto &s : stars_) {
    s.x = (uint16_t) irand(0, W_ - 1);
    s.y = (uint16_t) irand(0, H_ - 1);
    s.phase = frand(0.0f, 1.0f);
    s.speed = frand(TWINKLE_HZ_MIN, TWINKLE_HZ_MAX);
    s.max_b = (uint8_t) irand(120, 255);
  }
  shoot_active_ = false;
  shoot_timer_ = frand(SHOOT_MIN_S, SHOOT_MAX_S) * 0.5f;
}

void FxStarfield::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || stars_.empty())
    return;

  v.clear_black();

  for (auto &s : stars_) {
    s.phase += s.speed * dt;
    if (s.phase >= 1.0f)
      s.phase -= 1.0f;
    // Twinkle between ~37% and 100% of the star's max brightness
    const uint8_t tw = (uint8_t) (96 + ((159 * (int) sin8_[(uint8_t) (s.phase * 256.0f)]) >> 8));
    const uint8_t b = scale8(s.max_b, tw);
    // Slight blue-white tint
    v.put_px(s.x, s.y, fxutil::pack_rgb(b, b, fxutil::sat_add8(b, b >> 2)));
  }

  // Shooting star
  if (!shoot_active_) {
    shoot_timer_ -= dt;
    if (shoot_timer_ <= 0.0f) {
      shoot_active_ = true;
      const bool ltr = irand(0, 1) == 1;
      shoot_x_ = ltr ? -2.0f : (float) (W_ + 1);
      shoot_y_ = frand(0.0f, (float) H_ * 0.4f);
      const float speed = 2.2f * (float) W_;  // px/s, crosses in ~0.5s
      shoot_vx_ = ltr ? speed : -speed;
      shoot_vy_ = speed * frand(0.15f, 0.3f);  // shallow downward angle
    }
  } else {
    shoot_x_ += shoot_vx_ * dt;
    shoot_y_ += shoot_vy_ * dt;
    // Head plus a decaying trail sampled backwards along the velocity
    const float inv = 1.0f / std::sqrt(shoot_vx_ * shoot_vx_ + shoot_vy_ * shoot_vy_);
    const float ux = shoot_vx_ * inv, uy = shoot_vy_ * inv;
    for (int i = 0; i <= 5; ++i) {
      const uint8_t b = (uint8_t) (235 - i * 38);
      v.put_px_add((int) (shoot_x_ - ux * i), (int) (shoot_y_ - uy * i), b, b, fxutil::sat_add8(b, b >> 3));
    }
    if (shoot_x_ < -8.0f || shoot_x_ > (float) W_ + 8.0f || shoot_y_ > (float) H_ + 8.0f) {
      shoot_active_ = false;
      shoot_timer_ = frand(SHOOT_MIN_S, SHOOT_MAX_S);
    }
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
