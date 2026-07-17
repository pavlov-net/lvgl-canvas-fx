// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_ripples.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::scale8;

void FxRipples::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  for (auto &rp : ripples_)
    rp.active = false;
  spawn_timer_ = 0.3f;
}

// Midpoint circle: integer adds/compares only, additive plot so crossing
// rings brighten naturally.
void FxRipples::draw_ring_(const CanvasView &v, int cx, int cy, int r, uint8_t br) const {
  const uint8_t cr = scale8(RING_R, br), cg = scale8(RING_G, br), cb = scale8(RING_B, br);
  int x = r, y = 0, err = 1 - r;
  while (x >= y) {
    v.put_px_add(cx + x, cy + y, cr, cg, cb);
    v.put_px_add(cx - x, cy + y, cr, cg, cb);
    v.put_px_add(cx + x, cy - y, cr, cg, cb);
    v.put_px_add(cx - x, cy - y, cr, cg, cb);
    v.put_px_add(cx + y, cy + x, cr, cg, cb);
    v.put_px_add(cx - y, cy + x, cr, cg, cb);
    v.put_px_add(cx + y, cy - x, cr, cg, cb);
    v.put_px_add(cx - y, cy - x, cr, cg, cb);
    ++y;
    if (err < 0) {
      err += 2 * y + 1;
    } else {
      --x;
      err += 2 * (y - x) + 1;
    }
  }
}

void FxRipples::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || W_ <= 0 || H_ <= 0)
    return;

  v.fill(fxutil::pack_rgb(WATER_R, WATER_G, WATER_B));

  spawn_timer_ -= dt;
  if (spawn_timer_ <= 0.0f) {
    for (auto &rp : ripples_) {
      if (rp.active)
        continue;
      rp.active = true;
      rp.cx = frand(2.0f, (float) W_ - 3.0f);
      rp.cy = frand(2.0f, (float) H_ - 3.0f);
      rp.r = 0.5f;
      rp.max_r = frand(0.25f, 0.5f) * (float) std::min(W_, H_);
      rp.speed = frand(8.0f, 14.0f);
      break;
    }
    spawn_timer_ = frand(SPAWN_MIN_S, SPAWN_MAX_S);
  }

  for (auto &rp : ripples_) {
    if (!rp.active)
      continue;
    rp.r += rp.speed * dt;
    if (rp.r >= rp.max_r) {
      rp.active = false;
      continue;
    }
    // Ease brightness out as the ring expands (squared falloff)
    const float t = 1.0f - rp.r / rp.max_r;
    const uint8_t br = (uint8_t) (255.0f * t * t);
    draw_ring_(v, (int) rp.cx, (int) rp.cy, (int) rp.r, br);
    // Dimmer trailing echo ring
    const int r2 = (int) (rp.r * 0.55f);
    if (r2 >= 1 && r2 < (int) rp.r - 1)
      draw_ring_(v, (int) rp.cx, (int) rp.cy, r2, br >> 2);
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
