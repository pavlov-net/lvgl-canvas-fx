// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_matrix.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::frand;
using fxutil::irand;

void FxMatrix::respawn_(Drop &d) {
  // Stagger restarts above the canvas so columns stay desynchronized
  d.y = -frand(2.0f, (float) H_ * 1.5f);
  d.speed = frand(SPEED_MIN, SPEED_MAX);
}

void FxMatrix::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    drops_.clear();
    return;
  }
  drops_.resize((size_t) W_);
  for (auto &d : drops_) {
    respawn_(d);
    d.y = frand(-(float) H_, (float) H_);  // scatter initial positions
  }
}

void FxMatrix::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || drops_.empty())
    return;

  // The trail is entirely the previous frames fading out — long and smooth
  v.fade_to_black(TRAIL_FADE_OPA);

  for (int x = 0; x < W_; ++x) {
    Drop &d = drops_[x];
    const float y0 = d.y;
    d.y += d.speed * dt;
    if ((int) d.y > H_ + 2) {
      respawn_(d);
      continue;
    }
    // Paint every cell the head crossed this frame so trails have no gaps.
    // Head is a soft green-white, capped well below full brightness; the
    // cell behind it is pure phosphor green so the tail settles into hue.
    for (int y = (int) y0; y <= (int) d.y; ++y) {
      v.put_px(x, y - 1, fxutil::pack_rgb(24, 130, 52));
      v.put_px(x, y, fxutil::pack_rgb(120, 190, 130));
    }
  }

  // Occasional faint shimmer in the field, like distant glyphs changing
  shimmer_acc_ += (float) SHIMMER_PER_S * dt;
  while (shimmer_acc_ >= 1.0f) {
    shimmer_acc_ -= 1.0f;
    v.put_px_add(irand(0, W_ - 1), irand(0, H_ - 1), 8, 40, 14);
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
