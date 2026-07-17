// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Ambient "digital rain": soft green column heads with long smooth fade
// trails. Deliberately muted and slow — a calm terminal glow, not a strobe.
class FxMatrix : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr float SPEED_MIN = 7.0f;   // px/s
  static constexpr float SPEED_MAX = 22.0f;
  static constexpr uint8_t TRAIL_FADE_OPA = 18;  // long gentle tails
  static constexpr int SHIMMER_PER_S = 30;       // faint flickers/sec across canvas

  struct Drop {
    float y;
    float speed;
  };

  void respawn_(Drop &d);

  int W_{0}, H_{0};
  std::vector<Drop> drops_;  // one per column
  float shimmer_acc_{0};
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
