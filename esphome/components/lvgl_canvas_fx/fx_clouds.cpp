// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_clouds.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::GradStop;

void FxClouds::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0)
    return;
  col1_.assign((size_t) W_, 0);
  col2_.assign((size_t) W_, 0);
  row_.assign((size_t) H_, 0);
  diag_.assign((size_t) (W_ + H_), 0);
  fxutil::build_sin_lut_u8(sin8_);

  // Low-contrast night-sky grays with a bluish lavender in the thick parts
  static const GradStop STOPS[] = {
      {0, 10, 12, 22},     // clear dark sky
      {90, 28, 33, 52},    // thin haze
      {170, 62, 70, 98},   // cloud body
      {255, 105, 110, 138} // bright billow top
  };
  pal_.build(STOPS, 4);
}

void FxClouds::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || (int) col1_.size() != v.w || (int) row_.size() != v.h)
    return;

  t1_ += dt * DRIFT1;
  t2_ += dt * DRIFT2;
  ty_ += dt * BILLOW;
  td_ += dt * DIAG;

  const int o1 = (int) t1_, o2 = (int) t2_, oy = (int) ty_, od = (int) td_;
  for (int x = 0; x < v.w; ++x) {
    col1_[x] = sin8_[(x * SCALE_X1 + o1) & 255];
    col2_[x] = sin8_[(x * SCALE_X2 + o2) & 255];
  }
  for (int y = 0; y < v.h; ++y)
    row_[y] = sin8_[(y * SCALE_Y + oy) & 255];
  const int nd = v.w + v.h;
  for (int d = 0; d < nd; ++d)
    diag_[d] = sin8_[(d * SCALE_D + od) & 255];

  for (int y = 0; y < v.h; ++y) {
    const uint8_t ry = row_[y];
    const uint8_t *dg = &diag_[y];
#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      uint8_t idx = (uint8_t) (((int) col1_[x] + col2_[x] + ry + dg[x]) >> 2);
      p[0] = pal_.e[idx].r;
      p[1] = pal_.e[idx].g;
      p[2] = pal_.e[idx].b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    for (int x = 0; x < v.w; ++x) {
      uint8_t idx = (uint8_t) (((int) col1_[x] + col2_[x] + ry + dg[x]) >> 2);
      p[x] = pal_.e[idx];
    }
#endif
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
