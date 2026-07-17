// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Hyperspace warp: stars streak outward from the center, accelerating and
// brightening as they approach the edges.
class FxWarp : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int NUM_STARS = 40;
  static constexpr float GROW_RATE = 1.6f;   // exponential radial growth /s
  static constexpr float BASE_SPEED = 2.0f;  // px/s floor so center stars move
  static constexpr uint8_t TRAIL_FADE_OPA = 70;

  struct Star {
    float ux, uy;  // unit direction from center
    float r;       // distance from center
  };

  void respawn_(Star &s);

  int W_{0}, H_{0};
  float max_r_{1.0f};
  std::vector<Star> stars_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
