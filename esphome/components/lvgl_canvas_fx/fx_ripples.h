// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"

namespace esphome {
namespace lvgl_canvas_fx {

// Koi pond: concentric rings spawn at random points and expand outward,
// fading as they grow — raindrops on still water.
class FxRipples : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int MAX_RIPPLES = 6;
  static constexpr float SPAWN_MIN_S = 1.2f;
  static constexpr float SPAWN_MAX_S = 3.5f;
  static constexpr uint8_t RING_R = 90, RING_G = 170, RING_B = 200;
  static constexpr uint8_t WATER_R = 0, WATER_G = 10, WATER_B = 20;

  struct Ripple {
    bool active{false};
    float cx, cy, r, max_r, speed;
  };

  void draw_ring_(const fxutil::CanvasView &v, int cx, int cy, int r, uint8_t br) const;

  int W_{0}, H_{0};
  Ripple ripples_[MAX_RIPPLES];
  float spawn_timer_{0.5f};
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
