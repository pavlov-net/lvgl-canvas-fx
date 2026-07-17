// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Static star positions with slow independent brightness twinkle and the
// occasional shooting star.
class FxStarfield : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int STAR_DIV = 96;           // stars = w*h / STAR_DIV
  static constexpr float TWINKLE_HZ_MIN = 0.15f;
  static constexpr float TWINKLE_HZ_MAX = 0.5f;
  static constexpr float SHOOT_MIN_S = 8.0f;    // gap between shooting stars
  static constexpr float SHOOT_MAX_S = 20.0f;

  struct Star {
    uint16_t x, y;
    float phase, speed;
    uint8_t max_b;
  };

  int W_{0}, H_{0};
  uint8_t sin8_[256]{};
  std::vector<Star> stars_;

  bool shoot_active_{false};
  float shoot_x_{0}, shoot_y_{0}, shoot_vx_{0}, shoot_vy_{0};
  float shoot_timer_{4.0f};
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
