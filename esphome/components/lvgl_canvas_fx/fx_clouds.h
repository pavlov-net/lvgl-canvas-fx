// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Slow drifting cloud / fog banks: two sine octaves scrolling horizontally
// at different speeds (parallax) through a low-contrast gray-blue palette.
class FxClouds : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  // Scroll speeds in sine-LUT steps/sec (mostly horizontal drift)
  static constexpr float DRIFT1 = 6.0f;   // slow far layer
  static constexpr float DRIFT2 = 11.0f;  // faster near layer
  static constexpr float BILLOW = 2.5f;   // slow vertical churn
  static constexpr float DIAG = 1.8f;
  // Spatial frequencies (LUT steps per pixel)
  static constexpr int SCALE_X1 = 3;
  static constexpr int SCALE_X2 = 7;
  static constexpr int SCALE_Y = 4;
  static constexpr int SCALE_D = 2;

  int W_{0}, H_{0};
  float t1_{0}, t2_{0}, ty_{0}, td_{0};
  uint8_t sin8_[256]{};
  fxutil::Palette256 pal_;
  std::vector<uint8_t> col1_, col2_, row_, diag_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
