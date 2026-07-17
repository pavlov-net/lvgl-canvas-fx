// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Classic sum-of-sines plasma, slowed way down and run through a muted
// cyan/violet/indigo palette so it reads as drifting color clouds.
class FxPlasma : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  // Phase speeds (cycles/sec) — incommensurate so the pattern never loops
  static constexpr float SPEED1 = 0.043f;
  static constexpr float SPEED2 = 0.031f;
  static constexpr float SPEED3 = 0.023f;
  static constexpr float PAL_DRIFT = 2.5f;  // palette indices/sec
  // Spatial frequency (sine LUT steps per pixel)
  static constexpr int SCALE_X = 4;
  static constexpr int SCALE_Y = 5;
  static constexpr int SCALE_D = 3;

  void build_palette_();

  int W_{0}, H_{0};
  float t1_{0}, t2_{0}, t3_{0};
  float pal_pos_{0};
  uint8_t sin8_[256]{};
  fxutil::Palette256 pal_;
  std::vector<uint8_t> col_, row_, diag_;  // per-frame line LUTs
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
