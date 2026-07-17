// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_snowfall.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;

void FxSnowfall::respawn_(Flake &f, bool anywhere) {
  f.x = frand(0.0f, (float) W_);
  f.y = anywhere ? frand(0.0f, (float) H_) : frand(-4.0f, -1.0f);
  f.phase = frand(0.0f, 1.0f);
  if (f.layer == 1) {
    f.fall = frand(0.25f, 0.40f) * (float) H_;
  } else {
    f.fall = frand(0.10f, 0.18f) * (float) H_;
  }
}

void FxSnowfall::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    flakes_.clear();
    return;
  }
  fxutil::build_sin_lut_u8(sin8_);

  const int n = std::max(6, W_ * H_ / DENSITY_DIV);
  flakes_.resize((size_t) n);
  for (size_t i = 0; i < flakes_.size(); ++i) {
    flakes_[i].layer = (i < (size_t) (n * FG_FRACTION)) ? 1 : 0;
    respawn_(flakes_[i], true);
  }
}

void FxSnowfall::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || flakes_.empty())
    return;

  v.clear_black();

  for (auto &f : flakes_) {
    f.y += f.fall * dt;
    f.phase += (f.layer ? SWAY_HZ_FG : SWAY_HZ_BG) * dt;
    if (f.phase >= 1.0f)
      f.phase -= 1.0f;
    if (f.y > (float) H_ + 1.0f) {
      respawn_(f, false);
      continue;
    }

    const int amp = f.layer ? SWAY_AMP_FG : SWAY_AMP_BG;
    const int sway = (((int) sin8_[(uint8_t) (f.phase * 256.0f)] - 128) * amp) >> 7;
    const int x = (int) f.x + sway;
    const int y = (int) f.y;

    if (f.layer == 1) {
      // Bright foreground flake with a soft plus-shaped halo
      v.put_px_add(x, y, 235, 240, 255);
      v.put_px_add(x - 1, y, 90, 92, 100);
      v.put_px_add(x + 1, y, 90, 92, 100);
      v.put_px_add(x, y - 1, 90, 92, 100);
      v.put_px_add(x, y + 1, 90, 92, 100);
    } else {
      // Dim cool-gray background flake
      v.put_px_add(x, y, 82, 86, 104);
    }
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
