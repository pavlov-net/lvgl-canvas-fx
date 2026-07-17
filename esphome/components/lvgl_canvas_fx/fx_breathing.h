// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>
#include "esphome/core/helpers.h"

namespace esphome {
namespace lvgl_canvas_fx {

// Soft radial glow that pulses brightness on a slow sine — a calm,
// meditation-paced breathing light.
class FxBreathing : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr float PERIOD_S = 6.0f;     // full breath cycle
  static constexpr uint8_t MIN_BRIGHT = 25;   // floor so it never fully blacks out
  static constexpr uint8_t BASE_R = 255, BASE_G = 147, BASE_B = 41;  // warm amber
  static constexpr int GRAD_DEPTH = 215;      // 255-GRAD_DEPTH left at the corners

  int W_{0}, H_{0};
  float phase_{0};
  uint8_t sin8_[256]{};
  fxutil::NativePix lut_[256]{};
  std::vector<uint8_t, esphome::RAMAllocator<uint8_t>> base_;  // radial falloff map
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
