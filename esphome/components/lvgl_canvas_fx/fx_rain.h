// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"

namespace esphome {
namespace lvgl_canvas_fx {

// Rain on glass: fast falling streaks behind slow stick-slip droplets that
// cling, slide and merge. Intensity 0..1 is runtime-settable via
// submit_data() (a single float or byte) — 0 lets the rain drain away to
// nothing, so it can track real weather from Home Assistant.
class FxRain : public FxBase {
 public:
  void set_intensity(float v) { intensity_ = fxutil::clampf(v, 0.0f, 1.0f); }

  void on_resize(const Rect &r) override;
  void step(float dt) override;
  void on_data(const void *data, size_t bytes) override;

 private:
  static constexpr int MAX_STREAKS = 40;
  static constexpr int MAX_DROPLETS = 10;
  static constexpr float MAX_SPAWN_PER_S = 60.0f;  // streaks/s at intensity 1
  static constexpr float DROPLET_PER_S = 1.2f;     // droplets/s at intensity 1
  static constexpr uint8_t TRAIL_FADE_OPA = 90;

  struct Streak {
    bool active{false};
    float x, y, speed;
    uint8_t len;
  };
  struct Droplet {
    bool active{false};
    float x, y, vy;
    float stick_t;  // time until the next slide burst
    uint8_t size;
  };

  int W_{0}, H_{0};
  float intensity_{0.5f};
  float spawn_acc_{0};
  float droplet_acc_{0};
  Streak streaks_[MAX_STREAKS];
  Droplet droplets_[MAX_DROPLETS];
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
