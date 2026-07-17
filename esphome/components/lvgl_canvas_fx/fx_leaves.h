// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>

namespace esphome {
namespace lvgl_canvas_fx {

// Falling leaves (or cherry-blossom petals): slow tumbling particles with
// strong sideways flutter. Registered as both "leaves" and "sakura" with
// different palettes.
class FxLeaves : public FxBase {
 public:
  enum class Variant : uint8_t { AUTUMN, SAKURA };

  void set_variant(Variant v) { variant_ = v; }

  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int DENSITY_DIV = 260;   // leaves = w*h / DENSITY_DIV
  static constexpr float SWAY_HZ = 0.22f;
  static constexpr float TUMBLE_HZ_MIN = 0.3f;
  static constexpr float TUMBLE_HZ_MAX = 0.7f;

  struct Leaf {
    float x, y;
    float fall;        // px/s
    float sway_amp;    // px
    float phase;       // sway phase
    float tumble;      // tumble phase
    float tumble_hz;
    uint8_t r, g, b;
  };

  void respawn_(Leaf &l, bool anywhere);

  Variant variant_{Variant::AUTUMN};
  int W_{0}, H_{0};
  uint8_t sin8_[256]{};
  std::vector<Leaf> leaves_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
