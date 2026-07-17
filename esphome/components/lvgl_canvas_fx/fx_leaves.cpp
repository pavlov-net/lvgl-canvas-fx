// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_leaves.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::irand;
using fxutil::scale8;

namespace {
struct Rgb {
  uint8_t r, g, b;
};
const Rgb AUTUMN_COLORS[] = {{200, 90, 20}, {170, 60, 15}, {155, 105, 22}, {125, 42, 12}, {215, 135, 32}};
const Rgb SAKURA_COLORS[] = {{240, 170, 190}, {250, 200, 215}, {230, 140, 170}, {255, 225, 235}};
}  // namespace

void FxLeaves::respawn_(Leaf &l, bool anywhere) {
  l.x = frand(0.0f, (float) W_);
  l.y = anywhere ? frand(0.0f, (float) H_) : frand(-4.0f, -1.0f);
  l.fall = frand(0.07f, 0.14f) * (float) H_;
  l.sway_amp = frand(2.0f, 5.0f);
  l.phase = frand(0.0f, 1.0f);
  l.tumble = frand(0.0f, 1.0f);
  l.tumble_hz = frand(TUMBLE_HZ_MIN, TUMBLE_HZ_MAX);

  if (variant_ == Variant::SAKURA) {
    const Rgb &c = SAKURA_COLORS[irand(0, (int) (sizeof(SAKURA_COLORS) / sizeof(Rgb)) - 1)];
    l.r = c.r;
    l.g = c.g;
    l.b = c.b;
  } else {
    const Rgb &c = AUTUMN_COLORS[irand(0, (int) (sizeof(AUTUMN_COLORS) / sizeof(Rgb)) - 1)];
    l.r = c.r;
    l.g = c.g;
    l.b = c.b;
  }
}

void FxLeaves::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    leaves_.clear();
    return;
  }
  fxutil::build_sin_lut_u8(sin8_);

  const int n = std::max(4, W_ * H_ / DENSITY_DIV);
  leaves_.resize((size_t) n);
  for (auto &l : leaves_)
    respawn_(l, true);
}

void FxLeaves::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || leaves_.empty())
    return;

  v.clear_black();

  for (auto &l : leaves_) {
    l.y += l.fall * dt;
    l.phase += SWAY_HZ * dt;
    if (l.phase >= 1.0f)
      l.phase -= 1.0f;
    l.tumble += l.tumble_hz * dt;
    if (l.tumble >= 1.0f)
      l.tumble -= 1.0f;
    if (l.y > (float) H_ + 1.0f) {
      respawn_(l, false);
      continue;
    }

    const int sway = (((int) sin8_[(uint8_t) (l.phase * 256.0f)] - 128) * (int) (l.sway_amp * 2.0f)) >> 8;
    const int x = (int) l.x + sway;
    const int y = (int) l.y;

    // Tumble: brightness dips and the leaf flips between wide and tall as
    // it rotates edge-on
    const uint8_t ts = sin8_[(uint8_t) (l.tumble * 256.0f)];
    const uint8_t br = (uint8_t) (140 + ((115 * (int) ts) >> 8));
    const uint8_t r = scale8(l.r, br), g = scale8(l.g, br), b = scale8(l.b, br);

    v.put_px_add(x, y, r, g, b);
    if (ts > 170) {
      v.put_px_add(x + 1, y, r >> 1, g >> 1, b >> 1);  // face-on: wide
    } else if (ts < 86) {
      v.put_px_add(x, y + 1, r >> 1, g >> 1, b >> 1);  // edge-on: tall
    }
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
