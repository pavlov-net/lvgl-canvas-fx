// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_boids.h"
#include <cmath>

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::irand;

void FxBoids::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    boids_.clear();
    return;
  }
  boids_.resize(NUM_BOIDS);
  for (auto &b : boids_) {
    b.x = frand(2.0f, (float) W_ - 2.0f);
    b.y = frand(2.0f, (float) H_ - 2.0f);
    const float a = frand(0.0f, 2.0f * (float) M_PI);
    const float s = frand(VMIN, VMAX);
    b.vx = std::cos(a) * s;
    b.vy = std::sin(a) * s;
    b.hue = (uint8_t) irand(0, 2);
  }
}

void FxBoids::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || boids_.empty())
    return;

  v.fade_to_black(TRAIL_FADE_OPA);

  const int n = (int) boids_.size();
  for (int i = 0; i < n; ++i) {
    Boid &b = boids_[i];
    float sep_x = 0, sep_y = 0;
    float avg_vx = 0, avg_vy = 0;
    float cen_x = 0, cen_y = 0;
    int neigh = 0;

    for (int j = 0; j < n; ++j) {
      if (j == i)
        continue;
      const Boid &o = boids_[j];
      const float dx = o.x - b.x, dy = o.y - b.y;
      const float d2 = dx * dx + dy * dy;
      if (d2 > NEIGH_R2)
        continue;
      ++neigh;
      avg_vx += o.vx;
      avg_vy += o.vy;
      cen_x += o.x;
      cen_y += o.y;
      if (d2 < SEP_R2 && d2 > 0.01f) {
        sep_x -= dx / d2;
        sep_y -= dy / d2;
      }
    }

    float ax = sep_x * W_SEP, ay = sep_y * W_SEP;
    if (neigh > 0) {
      ax += (avg_vx / neigh - b.vx) * W_ALI;
      ay += (avg_vy / neigh - b.vy) * W_ALI;
      ax += (cen_x / neigh - b.x) * W_COH;
      ay += (cen_y / neigh - b.y) * W_COH;
    }
    // Steer away from edges
    if (b.x < EDGE_MARGIN)
      ax += W_EDGE * (1.0f - b.x / EDGE_MARGIN);
    if (b.x > (float) W_ - EDGE_MARGIN)
      ax -= W_EDGE * (1.0f - ((float) W_ - b.x) / EDGE_MARGIN);
    if (b.y < EDGE_MARGIN)
      ay += W_EDGE * (1.0f - b.y / EDGE_MARGIN);
    if (b.y > (float) H_ - EDGE_MARGIN)
      ay -= W_EDGE * (1.0f - ((float) H_ - b.y) / EDGE_MARGIN);

    b.vx += ax * dt;
    b.vy += ay * dt;

    // Keep speed inside [VMIN, VMAX]
    const float sp = std::sqrt(b.vx * b.vx + b.vy * b.vy);
    if (sp > VMAX) {
      b.vx *= VMAX / sp;
      b.vy *= VMAX / sp;
    } else if (sp > 0.01f && sp < VMIN) {
      b.vx *= VMIN / sp;
      b.vy *= VMIN / sp;
    }

    b.x = fxutil::clampf(b.x + b.vx * dt, 0.0f, (float) W_ - 1.0f);
    b.y = fxutil::clampf(b.y + b.vy * dt, 0.0f, (float) H_ - 1.0f);

    // Cool white with a per-boid tint; a dim pixel behind gives direction
    const uint8_t r = b.hue == 1 ? 200 : 170;
    const uint8_t g = b.hue == 2 ? 210 : 185;
    v.put_px_add((int) b.x, (int) b.y, r, g, 235);
    if (sp > 0.01f)
      v.put_px_add((int) (b.x - b.vx / sp), (int) (b.y - b.vy / sp), r >> 2, g >> 2, 235 >> 2);
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
