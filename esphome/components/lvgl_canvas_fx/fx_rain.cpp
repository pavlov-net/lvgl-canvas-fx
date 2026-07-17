// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_rain.h"
#include <cstring>
#include "esphome/core/log.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::irand;

static const char *const TAG = "fx_rain";

void FxRain::on_data(const void *data, size_t bytes) {
  if (bytes == sizeof(float)) {
    float f;
    memcpy(&f, data, sizeof(f));
    set_intensity(f);
  } else if (bytes == 1) {
    set_intensity((float) *(const uint8_t *) data / 255.0f);
  } else {
    ESP_LOGW(TAG, "on_data expects a float (0..1) or single byte, got %u bytes", (unsigned) bytes);
  }
}

void FxRain::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  for (auto &s : streaks_)
    s.active = false;
  for (auto &d : droplets_)
    d.active = false;
  spawn_acc_ = 0;
  droplet_acc_ = 0;
}

void FxRain::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || W_ <= 0 || H_ <= 0)
    return;

  // Short comet tails; also drains everything to black at intensity 0
  v.fade_to_black(TRAIL_FADE_OPA);

  // --- falling streaks ---
  spawn_acc_ += intensity_ * MAX_SPAWN_PER_S * dt;
  while (spawn_acc_ >= 1.0f) {
    spawn_acc_ -= 1.0f;
    for (auto &s : streaks_) {
      if (s.active)
        continue;
      s.active = true;
      s.x = frand(0.0f, (float) W_);
      s.speed = frand(0.9f, 1.6f) * (float) H_;
      s.len = (uint8_t) irand(3, 6);
      s.y = -(float) s.len;
      break;
    }
  }

  for (auto &s : streaks_) {
    if (!s.active)
      continue;
    s.y += s.speed * dt;
    if (s.y - s.len > (float) H_) {
      s.active = false;
      continue;
    }
    // Faster streaks are brighter (closer to the glass)
    const uint8_t head = (uint8_t) (120 + 90.0f * (s.speed / (float) H_ - 0.9f) / 0.7f);
    const int x = (int) s.x, y = (int) s.y;
    for (int i = 0; i < (int) s.len; ++i) {
      const uint8_t b = (uint8_t) ((head * (s.len - i)) / s.len);
      v.put_px_add(x, y - i, (uint8_t) ((b * 170) >> 8), (uint8_t) ((b * 200) >> 8), b);
    }
  }

  // --- clinging droplets (stick-slip) ---
  droplet_acc_ += intensity_ * DROPLET_PER_S * dt;
  while (droplet_acc_ >= 1.0f) {
    droplet_acc_ -= 1.0f;
    for (auto &d : droplets_) {
      if (d.active)
        continue;
      d.active = true;
      d.x = frand(1.0f, (float) W_ - 2.0f);
      d.y = frand(0.0f, (float) H_ * 0.6f);
      d.vy = 0;
      d.stick_t = frand(0.5f, 2.5f);
      d.size = 1;
      break;
    }
  }

  for (auto &d : droplets_) {
    if (!d.active)
      continue;
    if (d.vy > 0.0f) {
      // Sliding: decelerate back into a stick
      d.y += d.vy * dt;
      d.x += frand(-3.0f, 3.0f) * dt;
      d.vy -= 14.0f * dt;
      if (d.vy <= 0.0f) {
        d.vy = 0;
        d.stick_t = frand(0.4f, 2.0f);
      }
    } else {
      d.stick_t -= dt;
      if (d.stick_t <= 0.0f)
        d.vy = frand(4.0f, 10.0f) + (float) d.size;
    }
    if (d.y > (float) H_) {
      d.active = false;
      continue;
    }

    // Merge with any droplet close below/beside: survivor grows and slides
    for (auto &o : droplets_) {
      if (&o == &d || !o.active)
        continue;
      const float dx = o.x - d.x, dy = o.y - d.y;
      if (dx * dx + dy * dy <= 4.0f) {
        o.active = false;
        d.size = (uint8_t) std::min(3, d.size + o.size);
        d.vy = std::max(d.vy, frand(5.0f, 9.0f));
      }
    }

    // Bead body plus a bright highlight pixel
    const int x = (int) d.x, y = (int) d.y;
    for (int i = 0; i < (int) d.size; ++i) {
      v.put_px_add(x, y + i, 60, 90, 130);
      v.put_px_add(x + 1, y + i, 45, 65, 100);
    }
    v.put_px_add(x, y, 110, 135, 170);
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
