// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "fx_circle.h"
#include "fx_registry.h"
#include <math.h>
#include <algorithm>

extern "C" {
#include <lvgl.h>
}

namespace esphome {
namespace lvgl_canvas_fx {

void FxCircle::on_bind(lv_obj_t *canvas) {
  FxBase::on_bind(canvas);

  lv_draw_rect_dsc_init(&dsc_bg_);
  dsc_bg_.bg_color = lv_color_black();
  dsc_bg_.bg_opa = LV_OPA_COVER;
  dsc_bg_.border_opa = LV_OPA_TRANSP;

  lv_draw_rect_dsc_init(&dsc_fg_);
  dsc_fg_.bg_color = lv_palette_main(LV_PALETTE_BLUE);
  dsc_fg_.bg_opa = LV_OPA_COVER;
  dsc_fg_.border_opa = LV_OPA_TRANSP;
  dsc_fg_.radius = LV_RADIUS_CIRCLE;

  dsc_init_ = true;
  prev_r_ = -1;

  lv_canvas_fill_bg(canvas_, lv_color_black(), LV_OPA_COVER);  // clear canvas
}

void FxCircle::on_resize(const Rect &r) {
  FxBase::on_resize(r);
  prev_r_ = -1;  // reset tracking

  lv_canvas_fill_bg(canvas_, lv_color_black(), LV_OPA_COVER);  // clear canvas
}

#if FX_CIRCLE_FAST_CLEAR
// Black-only fast clear for true-color (LV_COLOR_DEPTH >= 16).
// Uses memset(0) per row and a single lv_obj_invalidate_area() for the union.
void FxCircle::fast_clear_rect_(int x, int y, int w, int h) {
  if (!canvas_)
    return;

  // Guard against palette/indexed modes. Black!=0 isn't guaranteed there.
  static_assert(LV_COLOR_DEPTH >= 16, "FX_CIRCLE_FAST_CLEAR assumes true-color formats (>=16-bit).");

  // Clamp to canvas bounds
  const int cw = lv_obj_get_width(canvas_);
  const int ch = lv_obj_get_height(canvas_);
  int x0 = std::max(0, x);
  int y0 = std::max(0, y);
  int x1 = std::min(cw, x + w);
  int y1 = std::min(ch, y + h);
  if (x0 >= x1 || y0 >= y1)
    return;

  // Access backing buffer; black is all-zero for >=16-bit true-color formats.
  lv_draw_buf_t *draw_buf = lv_canvas_get_draw_buf(canvas_);
  if (!draw_buf || !draw_buf->data) {
    // Fallback (rare): draw a black rect via LVGL layer pipeline.
    lv_draw_rect_dsc_t bg;
    lv_draw_rect_dsc_init(&bg);
    bg.bg_color = lv_color_black();
    bg.bg_opa = LV_OPA_COVER;
    bg.border_opa = LV_OPA_TRANSP;
    lv_layer_t layer;
    lv_canvas_init_layer(canvas_, &layer);
    lv_area_t area;
    lv_area_set(&area, x0, y0, x1 - 1, y1 - 1);
    lv_draw_rect(&layer, &bg, &area);
    lv_canvas_finish_layer(canvas_, &layer);
    return;
  }

  constexpr int BPP = LV_COLOR_DEPTH / 8;  // 2 for 565, 3 for 888, 4 for 8888
  uint8_t *buf = draw_buf->data;
  const int stride_bytes = (int) draw_buf->header.stride;
  const int row_bytes = (x1 - x0) * BPP;

  for (int yy = y0; yy < y1; ++yy) {
    memset(buf + yy * stride_bytes + x0 * BPP, 0, row_bytes);
  }

  // Invalidate the union once (avoid per-row invalidates)
  lv_area_t a;
  lv_area_set(&a, (int32_t) x0, (int32_t) y0, (int32_t) (x1 - 1), (int32_t) (y1 - 1));
  lv_obj_invalidate_area(canvas_, &a);
}
#endif

void FxCircle::step(float dt) {
  if (!canvas_)
    return;
  if (!dsc_init_)
    on_bind(canvas_);

  t_ += dt;

  const int maxr = std::max(1, std::min(area_.w, area_.h) / 2 - 1);
  constexpr float TAU = 6.283185307179586f;
  const float phase = sinf(t_ * TAU * 0.5f);  // 0.5 Hz
  const int r = 4 + (int) lroundf(0.5f * (1.0f + phase) * (maxr - 4));

  const int cx = area_.x + area_.w / 2;
  const int cy = area_.y + area_.h / 2;
  const int side = r * 2;
  const int new_x = cx - r;
  const int new_y = cy - r;

  if (prev_r_ < 0 || r >= prev_r_) {
    // First frame or growing → just overdraw (no clear)
    lv_layer_t layer;
    lv_canvas_init_layer(canvas_, &layer);
    lv_area_t fg_area;
    lv_area_set(&fg_area, new_x, new_y, new_x + side - 1, new_y + side - 1);
    lv_draw_rect(&layer, &dsc_fg_, &fg_area);
    lv_canvas_finish_layer(canvas_, &layer);
  } else {
    // Shrinking → clear old bounds then draw
#if FX_CIRCLE_FAST_CLEAR
    fast_clear_rect_(prev_cx_ - prev_r_, prev_cy_ - prev_r_, prev_r_ * 2, prev_r_ * 2);
#else
    {
      lv_layer_t layer;
      lv_canvas_init_layer(canvas_, &layer);
      lv_area_t bg_area;
      lv_area_set(&bg_area, prev_cx_ - prev_r_, prev_cy_ - prev_r_,
                   prev_cx_ + prev_r_ - 1, prev_cy_ + prev_r_ - 1);
      lv_draw_rect(&layer, &dsc_bg_, &bg_area);
      lv_canvas_finish_layer(canvas_, &layer);
    }
#endif
    lv_layer_t layer;
    lv_canvas_init_layer(canvas_, &layer);
    lv_area_t fg_area;
    lv_area_set(&fg_area, new_x, new_y, new_x + side - 1, new_y + side - 1);
    lv_draw_rect(&layer, &dsc_fg_, &fg_area);
    lv_canvas_finish_layer(canvas_, &layer);
  }

  prev_cx_ = cx;
  prev_cy_ = cy;
  prev_r_ = r;
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
