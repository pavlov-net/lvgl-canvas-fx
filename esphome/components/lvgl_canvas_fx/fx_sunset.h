// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"

namespace esphome {
namespace lvgl_canvas_fx {

// Slowly shifting vertical sky gradient cycling night → dawn → day → dusk.
// Self-advances by default; send a float 0..1 via submit_data() to pin the
// phase externally (e.g. map Home Assistant sun elevation / time of day).
// Phase 0.0 = midnight, 0.25 = dawn, 0.5 = midday, 0.75 = dusk.
class FxSunset : public FxBase {
 public:
  void step(float dt) override;
  void on_data(const void *data, size_t bytes) override;

 private:
  static constexpr float CYCLE_S = 240.0f;      // free-running full cycle
  static constexpr float TRACK_RATE = 0.25f;    // per-sec approach to external target
  static constexpr int KEYS = 4;                // night, dawn, day, dusk
  static constexpr int ROWS = 4;                // gradient control points top→bottom

  // [keyframe][row control point][rgb]
  static const uint8_t SKY[KEYS][ROWS][3];

  float phase_{0.0f};
  float target_{-1.0f};  // <0 → free-running
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
