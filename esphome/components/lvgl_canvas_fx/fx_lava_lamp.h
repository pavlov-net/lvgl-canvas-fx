// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#pragma once
#include "fx_base.h"
#include "fx_util.h"
#include <vector>
#include "esphome/core/helpers.h"

namespace esphome {
namespace lvgl_canvas_fx {

// Lava lamp: metaball blobs drifting and merging. The field is computed at
// half resolution with integer squared-distance math (no sqrt) and mapped
// through a palette whose knee acts as the blob threshold.
class FxLavaLamp : public FxBase {
 public:
  void on_resize(const Rect &r) override;
  void step(float dt) override;

 private:
  static constexpr int NUM_BLOBS = 3;
  static constexpr int DOWNSCALE = 2;
  static constexpr float BLOB_R_FRAC = 0.30f;  // of min(w,h)
  static constexpr int FIELD_GAIN = 110;       // field value at the blob edge
  static constexpr float BUOY_HZ_MIN = 0.05f;
  static constexpr float BUOY_HZ_MAX = 0.09f;

  struct Blob {
    float x, y;      // full-res coords
    float vx;        // px/s lateral drift
    float phase;     // buoyancy phase
    float phase_hz;
    float r;         // radius, px
  };

  int W_{0}, H_{0}, FW_{0}, FH_{0};
  uint8_t sin8_[256]{};
  Blob blobs_[NUM_BLOBS];
  std::vector<uint8_t, esphome::RAMAllocator<uint8_t>> field_;  // FW_*FH_
  fxutil::Palette256 pal_;
};

}  // namespace lvgl_canvas_fx
}  // namespace esphome
