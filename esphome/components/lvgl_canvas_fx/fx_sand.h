// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>
#include "esphome/core/helpers.h"

namespace esphome {
namespace lvgl_canvas_fx {

// Falling-sand toy: a slowly hue-shifting stream of grains pours from the
// top and piles up; when the pile gets tall the sand gently drains away.
class FxSand : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int SUBSTEPS = 2;          // sim steps per frame
  static constexpr int GRAINS_PER_S = 55;     // emit rate
  static constexpr float HUE_DRIFT = 10.0f;   // palette indices/sec
  static constexpr float FILL_LIMIT = 0.55f;  // start draining at this fill
  static constexpr float DRAIN_ROWS_PER_S = 10.0f;

  void sim_substep_();

  // Cheap xorshift PRNG: esp_random() is rate-limited hardware entropy and
  // far too slow to call once per settled grain per frame.
  uint32_t rng_next_() {
    rng_ ^= rng_ << 13;
    rng_ ^= rng_ >> 17;
    rng_ ^= rng_ << 5;
    return rng_;
  }
  uint32_t rng_{0x9E3779B9u};

  int W_{0}, H_{0};
  std::vector<uint8_t, esphome::RAMAllocator<uint8_t>> grid_;  // 0 = empty, else palette idx
  fxutil::Palette256 pal_;
  float hue_pos_{1.0f};
  float emit_acc_{0};
  float drain_acc_{0};
  int grain_count_{0};
  bool draining_{false};
  bool flip_{false};  // alternate sweep direction to avoid bias
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
