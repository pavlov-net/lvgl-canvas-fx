// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_sand.h"

namespace esphome {
namespace lvgl_canvas_fx {

using fxutil::CanvasView;
using fxutil::GradStop;
using fxutil::irand;

void FxSand::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  W_ = r.w;
  H_ = r.h;
  if (W_ <= 0 || H_ <= 0) {
    grid_.clear();
    return;
  }
  grid_.assign((size_t) W_ * H_, 0);
  grain_count_ = 0;
  draining_ = false;
  rng_ ^= esp_random();  // seed the fast PRNG once

  // Soft pastel color wheel for the grains (cyclical; index 0 is unused —
  // it means "empty" in the grid)
  static const GradStop STOPS[] = {
      {0, 205, 120, 110},   // dusty coral
      {51, 205, 175, 105},  // sand gold
      {102, 130, 190, 130}, // sage
      {153, 110, 160, 200}, // dusty blue
      {204, 170, 125, 195}, // mauve
      {255, 205, 120, 110}, // wrap to coral
  };
  pal_.build(STOPS, 6);
}

void FxSand::sim_substep_() {
  // Bottom-up scan: a grain falls straight down, else tries the diagonals.
  // Alternate horizontal sweep direction to keep piles symmetric.
  flip_ = !flip_;
  for (int y = H_ - 2; y >= 0; --y) {
    uint8_t *row = &grid_[(size_t) y * W_];
    uint8_t *below = &grid_[(size_t) (y + 1) * W_];
    for (int i = 0; i < W_; ++i) {
      const int x = flip_ ? (W_ - 1 - i) : i;
      const uint8_t g = row[x];
      if (g == 0)
        continue;
      if (below[x] == 0) {
        below[x] = g;
        row[x] = 0;
        continue;
      }
      // Fast path: fully settled grains (both diagonals blocked) need no RNG
      const bool lfree = x > 0 && below[x - 1] == 0 && row[x - 1] == 0;
      const bool rfree = x < W_ - 1 && below[x + 1] == 0 && row[x + 1] == 0;
      if (!lfree && !rfree)
        continue;
      const int dir = (lfree && rfree) ? ((rng_next_() & 1) ? 1 : -1) : (lfree ? -1 : 1);
      below[x + dir] = g;
      row[x] = 0;
    }
  }
}

void FxSand::step(float dt) {
  auto v = CanvasView::acquire(canvas_, area_);
  if (!v.ok() || grid_.size() != (size_t) v.w * v.h)
    return;

  hue_pos_ += HUE_DRIFT * dt;
  if (hue_pos_ >= 256.0f)
    hue_pos_ -= 255.0f;  // stay in 1..255 (0 = empty)

  if (!draining_) {
    // Emit new grains near the top center with a little jitter
    emit_acc_ += (float) GRAINS_PER_S * dt;
    while (emit_acc_ >= 1.0f) {
      emit_acc_ -= 1.0f;
      const int x = fxutil::clampi(W_ / 2 + irand(-W_ / 6, W_ / 6), 0, W_ - 1);
      if (grid_[(size_t) x] == 0) {
        grid_[(size_t) x] = (uint8_t) std::max(1, (int) hue_pos_);
        ++grain_count_;
      }
    }
    if (grain_count_ > (int) (FILL_LIMIT * (float) (W_ * H_)))
      draining_ = true;
  } else {
    // Gently drain: shift everything down one row, discarding the bottom
    drain_acc_ += DRAIN_ROWS_PER_S * dt;
    while (drain_acc_ >= 1.0f) {
      drain_acc_ -= 1.0f;
      for (int x = 0; x < W_; ++x) {
        if (grid_[(size_t) (H_ - 1) * W_ + x] != 0)
          --grain_count_;
      }
      memmove(&grid_[(size_t) W_], &grid_[0], (size_t) (H_ - 1) * W_);
      memset(&grid_[0], 0, (size_t) W_);
      if (grain_count_ <= 0) {
        grain_count_ = 0;
        draining_ = false;
        break;
      }
    }
  }

  for (int s = 0; s < SUBSTEPS; ++s)
    sim_substep_();

  // Draw the whole grid through the palette
  const fxutil::NativePix bg = fxutil::pack_rgb(4, 4, 8);
  for (int y = 0; y < v.h; ++y) {
    const uint8_t *src = &grid_[(size_t) y * v.w];
#if LV_COLOR_DEPTH == 24
    uint8_t *p = v.row(y);
    for (int x = 0; x < v.w; ++x, p += 3) {
      const fxutil::NativePix c = src[x] ? pal_.e[src[x]] : bg;
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
#else
    auto *p = reinterpret_cast<fxutil::NativePix *>(v.row(y));
    for (int x = 0; x < v.w; ++x)
      p[x] = src[x] ? pal_.e[src[x]] : bg;
#endif
  }

  v.invalidate(canvas_);
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
