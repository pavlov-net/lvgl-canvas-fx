// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_plasma.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::GradStop;

void FxPlasma::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0)
    return;
  col_.assign((size_t) W_, 0);
  row_.assign((size_t) H_, 0);
  diag_.assign((size_t) (W_ + H_), 0);
  fxutil::build_sin_lut_u8(sin8_);
  build_palette_();
}

void FxPlasma::build_palette_() {
  // Muted aurora-adjacent wash; cyclical (first == last) because the
  // palette index wraps as it drifts.
  static const GradStop STOPS[] = {
      {0, 10, 0, 40},     // deep indigo
      {64, 0, 85, 95},    // dim teal
      {128, 85, 40, 135}, // soft violet
      {192, 15, 45, 105}, // slate blue
      {255, 10, 0, 40},   // back to indigo
  };
  pal_.build(STOPS, 5);
}

void FxPlasma::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || (int) col_.size() != v.w || (int) row_.size() != v.h)
    return;

  t1_ += dt * SPEED1;
  t2_ += dt * SPEED2;
  t3_ += dt * SPEED3;
  pal_pos_ += dt * PAL_DRIFT;
  if (pal_pos_ >= 256.0f)
    pal_pos_ -= 256.0f;

  const int o1 = (int) (t1_ * 256.0f);
  const int o2 = (int) (t2_ * 256.0f);
  const int o3 = (int) (t3_ * 256.0f);
  for (int x = 0; x < v.w; ++x)
    col_[x] = sin8_[(x * SCALE_X + o1) & 255];
  for (int y = 0; y < v.h; ++y)
    row_[y] = sin8_[(y * SCALE_Y + o2) & 255];
  const int nd = v.w + v.h;
  for (int d = 0; d < nd; ++d)
    diag_[d] = sin8_[(d * SCALE_D + o3) & 255];

  const uint8_t shift = (uint8_t) pal_pos_;
  for (int y = 0; y < v.h; ++y) {
    const uint8_t ry = row_[y];
    const uint8_t *dg = &diag_[y];
#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      uint8_t idx = (uint8_t) ((((int) col_[x] + ry + dg[x]) * 85) >> 8) + shift;
      p[0] = pal_.e[idx].r;
      p[1] = pal_.e[idx].g;
      p[2] = pal_.e[idx].b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    for (int x = 0; x < v.w; ++x) {
      uint8_t idx = (uint8_t) ((((int) col_[x] + ry + dg[x]) * 85) >> 8) + shift;
      p[x] = pal_.e[idx];
    }
#endif
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
