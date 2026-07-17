// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// A tiny murmuration: classic boids (separation / alignment / cohesion)
// drifting around the canvas with faint trails.
class FxBoids : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int NUM_BOIDS = 16;
  static constexpr float VMIN = 6.0f;   // px/s
  static constexpr float VMAX = 14.0f;
  static constexpr float SEP_R2 = 3.0f * 3.0f;
  static constexpr float NEIGH_R2 = 9.0f * 9.0f;
  static constexpr float W_SEP = 30.0f;
  static constexpr float W_ALI = 2.0f;
  static constexpr float W_COH = 1.2f;
  static constexpr float W_EDGE = 40.0f;
  static constexpr float EDGE_MARGIN = 6.0f;
  static constexpr uint8_t TRAIL_FADE_OPA = 60;

  struct Boid {
    float x, y, vx, vy;
    uint8_t hue;  // small per-boid tint variation
  };

  int W_{0}, H_{0};
  std::vector<Boid> boids_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
