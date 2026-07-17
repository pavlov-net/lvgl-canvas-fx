// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Soft snowfall with sine-based sway and two parallax depth layers:
// dim slow background flakes and bright soft foreground flakes.
class FxSnowfall : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int DENSITY_DIV = 140;   // flakes = w*h / DENSITY_DIV
  static constexpr float FG_FRACTION = 0.4f;
  static constexpr float SWAY_HZ_BG = 0.10f;
  static constexpr float SWAY_HZ_FG = 0.16f;
  static constexpr int SWAY_AMP_BG = 1;     // px
  static constexpr int SWAY_AMP_FG = 3;

  struct Flake {
    float x, y;      // base position (sway is added at draw time)
    float fall;      // px/s
    float phase;     // sway phase 0..1
    uint8_t layer;   // 0 = background, 1 = foreground
  };

  void respawn_(Flake &f, bool anywhere);

  int W_{0}, H_{0};
  uint8_t sin8_[256]{};
  std::vector<Flake> flakes_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
