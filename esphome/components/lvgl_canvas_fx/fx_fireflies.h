// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Warm glowing dots wandering slowly on a random walk, each breathing
// in and out on its own rhythm. Soft afterglow trails.
class FxFireflies : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int NUM_FLIES = 12;
  static constexpr float VMAX = 4.0f;          // px/s
  static constexpr float WANDER_ACCEL = 6.0f;  // px/s^2 random jitter
  static constexpr float PULSE_HZ_MIN = 0.2f;
  static constexpr float PULSE_HZ_MAX = 0.5f;
  static constexpr uint8_t TRAIL_FADE_OPA = 40;

  struct Fly {
    float x, y, vx, vy;
    float phase, pulse_hz;
  };

  int W_{0}, H_{0};
  uint8_t sin8_[256]{};
  std::vector<Fly> flies_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
