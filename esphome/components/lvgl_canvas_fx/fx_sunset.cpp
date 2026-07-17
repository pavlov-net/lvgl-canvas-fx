// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_sunset.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::clampf;
using fxutil::lerpf;

// Sky colors top → horizon for each keyframe
const uint8_t FxSunset::SKY[FxSunset::KEYS][FxSunset::ROWS][3] = {
    // night
    {{2, 2, 16}, {6, 7, 34}, {14, 13, 52}, {26, 22, 64}},
    // dawn
    {{24, 20, 62}, {88, 44, 92}, {198, 92, 82}, {255, 152, 92}},
    // day
    {{38, 88, 178}, {80, 138, 218}, {138, 188, 238}, {198, 222, 248}},
    // dusk
    {{16, 12, 48}, {78, 34, 86}, {198, 78, 58}, {255, 118, 58}},
};

void FxSunset::on_data(const void *data, size_t bytes) {
  if (bytes == sizeof(float)) {
    float f;
    memcpy(&f, data, sizeof(f));
    target_ = clampf(f, 0.0f, 1.0f);
  } else if (bytes == 1) {
    target_ = (float) *(const uint8_t *) data / 255.0f;
  } else {
    ESP_LOGW("fx_sunset", "on_data expects a float (0..1) or single byte, got %u bytes", (unsigned) bytes);
  }
}

void FxSunset::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok())
    return;

  if (target_ < 0.0f) {
    phase_ += dt / CYCLE_S;
    if (phase_ >= 1.0f)
      phase_ -= 1.0f;
  } else {
    // Ease toward the external target along the shortest wrap direction
    float d = target_ - phase_;
    if (d > 0.5f)
      d -= 1.0f;
    if (d < -0.5f)
      d += 1.0f;
    phase_ += d * clampf(TRACK_RATE * dt, 0.0f, 1.0f);
    if (phase_ >= 1.0f)
      phase_ -= 1.0f;
    if (phase_ < 0.0f)
      phase_ += 1.0f;
  }

  // Blend between the two surrounding keyframes
  const float kp = phase_ * KEYS;
  const int k1 = ((int) kp) % KEYS;
  const int k2 = (k1 + 1) % KEYS;
  const float kt = kp - (float) (int) kp;

  for (int y = 0; y < v.h; ++y) {
    // Position within the vertical gradient (0=top control point, ROWS-1=bottom)
    const float ry = (v.h > 1) ? (float) y / (float) (v.h - 1) * (ROWS - 1) : 0.0f;
    const int r1 = (int) ry;
    const int r2 = (r1 + 1 < ROWS) ? r1 + 1 : r1;
    const float rt = ry - (float) r1;

    uint8_t rgb[3];
    for (int c = 0; c < 3; ++c) {
      const float a = lerpf((float) SKY[k1][r1][c], (float) SKY[k1][r2][c], rt);
      const float b = lerpf((float) SKY[k2][r1][c], (float) SKY[k2][r2][c], rt);
      rgb[c] = (uint8_t) lerpf(a, b, kt);
    }
    const fxutil::NativePix c = fxutil::pack_rgb(rgb[0], rgb[1], rgb[2]);

#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    std::fill(p, p + v.w, c);
#endif
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
