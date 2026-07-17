// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Brooding storm sky: a dark blue-gray gradient with rare branching
// lightning bolts and a decaying full-canvas flash.
class FxLightning : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr float BOLT_MIN_S = 5.0f;
  static constexpr float BOLT_MAX_S = 15.0f;
  static constexpr float BOLT_TTL_S = 0.22f;  // bolt visible time
  static constexpr float FLASH_DECAY = 9.0f;  // /s exponential decay

  struct Pt {
    int16_t x, y;
  };

  void fire_bolt_();

  int W_{0}, H_{0};
  std::vector<uint8_t> base_r_, base_g_, base_b_;  // per-row sky gradient
  std::vector<Pt> bolt_, branch_;
  float bolt_timer_{3.0f};
  float bolt_ttl_{0};
  float flash_{0};
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
