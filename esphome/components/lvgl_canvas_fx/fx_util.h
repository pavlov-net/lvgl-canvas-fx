// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT
//
// Shared helpers for canvas effects: native pixel packing, a per-frame
// CanvasView over the draw buffer, palette building and small math utils.

#pragma once
#include "fx_base.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

extern "C" {
#include <lvgl.h>
}

#include "esp_random.h"

namespace esphome {
namespace lvgl_canvas_fx {
namespace fxutil {

// -------------------- native pixel --------------------

#if LV_COLOR_DEPTH == 16
using NativePix = uint16_t;
inline NativePix pack_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (NativePix) (((uint16_t) (r >> 3) << 11) | ((uint16_t) (g >> 2) << 5) | (b >> 3));
}
#elif LV_COLOR_DEPTH == 32
using NativePix = uint32_t;
inline NativePix pack_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return (0xFFu << 24) | ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
}
#else  // 24-bit RGB888: 3 bytes per pixel
struct NativePix {
  uint8_t r{0}, g{0}, b{0};
};
inline NativePix pack_rgb(uint8_t r, uint8_t g, uint8_t b) { return NativePix{r, g, b}; }
#endif

// -------------------- small math / random helpers --------------------

inline float frand(float a, float b) { return a + (b - a) * (float) (esp_random() & 0xFFFF) / 65535.0f; }
inline int irand(int lo, int hi) { return lo + (int) (esp_random() % (uint32_t) (hi - lo + 1)); }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline uint8_t scale8(uint8_t v, uint8_t s) { return (uint8_t) (((uint16_t) v * (uint16_t) s) >> 8); }
inline uint8_t sat_add8(uint8_t a, uint8_t b) {
  uint16_t s = (uint16_t) a + b;
  return s > 255 ? 255 : (uint8_t) s;
}

// out[i] = 128 + 127*sin(2*pi*i/256)
inline void build_sin_lut_u8(uint8_t out[256]) {
  for (int i = 0; i < 256; ++i) {
    float s = std::sin((float) i * (2.0f * (float) M_PI / 256.0f));
    out[i] = (uint8_t) std::lround(128.0f + 127.0f * s);
  }
}

// -------------------- gradient palette --------------------

struct GradStop {
  uint8_t pos;  // 0..255
  uint8_t r, g, b;
};

struct Palette256 {
  NativePix e[256]{};

  // Piecewise-linear gradient through `n` stops (stops sorted by pos,
  // first should be pos 0 and last pos 255 for full coverage).
  void build(const GradStop *stops, int n) {
    if (n <= 0)
      return;
    for (int i = 0; i < 256; ++i) {
      // Find surrounding stops
      int s1 = 0;
      while (s1 + 1 < n && stops[s1 + 1].pos <= i)
        ++s1;
      int s2 = (s1 + 1 < n) ? s1 + 1 : s1;
      int span = (int) stops[s2].pos - (int) stops[s1].pos;
      float t = (span > 0) ? (float) (i - stops[s1].pos) / (float) span : 0.0f;
      auto lerp8 = [&](uint8_t a, uint8_t b) { return (uint8_t) std::lround(lerpf((float) a, (float) b, t)); };
      e[i] = pack_rgb(lerp8(stops[s1].r, stops[s2].r), lerp8(stops[s1].g, stops[s2].g),
                      lerp8(stops[s1].b, stops[s2].b));
    }
  }
};

// -------------------- per-frame canvas view --------------------
//
// Acquire at the top of every step(); wraps the draw buffer with the effect's
// sub-rect (area) applied, so all coordinates below are area-relative.

struct CanvasView {
  uint8_t *buf{nullptr};
  int stride{0};  // bytes per canvas row
  int w{0}, h{0};
  int x0{0}, y0{0};

  static CanvasView acquire(lv_obj_t *canvas, const FxBase::Rect &area) {
    CanvasView v;
    if (!canvas || area.w <= 0 || area.h <= 0)
      return v;
    lv_draw_buf_t *db = lv_canvas_get_draw_buf(canvas);
    if (!db || !db->data)
      return v;
    // Validate the buffer matches the compile-time pixel layout
    lv_color_format_t cf = (lv_color_format_t) db->header.cf;
#if LV_COLOR_DEPTH == 16
    if (cf != LV_COLOR_FORMAT_RGB565)
      return v;
#elif LV_COLOR_DEPTH == 32
    if (cf != LV_COLOR_FORMAT_XRGB8888 && cf != LV_COLOR_FORMAT_ARGB8888)
      return v;
#else
    if (cf != LV_COLOR_FORMAT_RGB888)
      return v;
#endif
    v.buf = db->data;
    v.stride = (int) db->header.stride;
    v.w = area.w;
    v.h = area.h;
    v.x0 = area.x;
    v.y0 = area.y;
    return v;
  }

  bool ok() const { return buf != nullptr && w > 0 && h > 0; }

  static constexpr int BPP = LV_COLOR_DEPTH == 24 ? 3 : LV_COLOR_DEPTH / 8;

  uint8_t *row(int y) const { return buf + (size_t) (y0 + y) * stride + (size_t) x0 * BPP; }

  NativePix *px_ptr(int x, int y) const { return reinterpret_cast<NativePix *>(row(y)) + x; }

  void put_px(int x, int y, NativePix c) const {
    if ((unsigned) x >= (unsigned) w || (unsigned) y >= (unsigned) h)
      return;
#if LV_COLOR_DEPTH == 24
    uint8_t *p = row(y) + x * 3;
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
#else
    reinterpret_cast<NativePix *>(row(y))[x] = c;
#endif
  }

  // Saturating additive blend of (r,g,b) onto the existing pixel.
  void put_px_add(int x, int y, uint8_t r, uint8_t g, uint8_t b) const {
    if ((unsigned) x >= (unsigned) w || (unsigned) y >= (unsigned) h)
      return;
#if LV_COLOR_DEPTH == 16
    uint16_t *p = reinterpret_cast<uint16_t *>(row(y)) + x;
    uint16_t c = *p;
    uint8_t cr = (uint8_t) ((c >> 11) << 3);
    uint8_t cg = (uint8_t) (((c >> 5) & 0x3F) << 2);
    uint8_t cb = (uint8_t) ((c & 0x1F) << 3);
    *p = pack_rgb(sat_add8(cr, r), sat_add8(cg, g), sat_add8(cb, b));
#elif LV_COLOR_DEPTH == 32
    uint32_t *p = reinterpret_cast<uint32_t *>(row(y)) + x;
    uint32_t c = *p;
    *p = pack_rgb(sat_add8((uint8_t) (c >> 16), r), sat_add8((uint8_t) (c >> 8), g), sat_add8((uint8_t) c, b));
#else
    uint8_t *p = row(y) + x * 3;
    p[0] = sat_add8(p[0], r);
    p[1] = sat_add8(p[1], g);
    p[2] = sat_add8(p[2], b);
#endif
  }

  void fill(NativePix c) const {
    for (int y = 0; y < h; ++y) {
#if LV_COLOR_DEPTH == 24
      uint8_t *p = row(y);
      for (int x = 0; x < w; ++x, p += 3) {
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
      }
#else
      NativePix *p = reinterpret_cast<NativePix *>(row(y));
      std::fill(p, p + w, c);
#endif
    }
  }

  // Black is all-zero bytes in RGB565 / RGB888; keep alpha 0xFF for 32-bit.
  void clear_black() const {
#if LV_COLOR_DEPTH == 32
    fill(pack_rgb(0, 0, 0));
#else
    for (int y = 0; y < h; ++y)
      memset(row(y), 0x00, (size_t) w * BPP);
#endif
  }

  // Multiply every pixel toward black; opa==255 clears fully.
  void fade_to_black(uint8_t opa) const {
    if (opa == 0)
      return;
    if (opa >= 255) {
      clear_black();
      return;
    }
    const uint32_t a = 255u - opa;
    for (int y = 0; y < h; ++y) {
#if LV_COLOR_DEPTH == 16
      uint16_t *p = reinterpret_cast<uint16_t *>(row(y));
      for (int x = 0; x < w; ++x) {
        uint16_t c = p[x];
        uint32_t r = ((c >> 11) & 0x1F) * a >> 8;
        uint32_t g = ((c >> 5) & 0x3F) * a >> 8;
        uint32_t b = (c & 0x1F) * a >> 8;
        p[x] = (uint16_t) ((r << 11) | (g << 5) | b);
      }
#elif LV_COLOR_DEPTH == 32
      uint32_t *p = reinterpret_cast<uint32_t *>(row(y));
      for (int x = 0; x < w; ++x) {
        uint32_t c = p[x];
        uint32_t r = ((c >> 16) & 0xFF) * a >> 8;
        uint32_t g = ((c >> 8) & 0xFF) * a >> 8;
        uint32_t b = (c & 0xFF) * a >> 8;
        p[x] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
      }
#else
      uint8_t *p = row(y);
      const int nb = w * 3;
      for (int i = 0; i < nb; ++i)
        p[i] = (uint8_t) ((p[i] * a) >> 8);
#endif
    }
  }

  void invalidate(lv_obj_t *canvas) const {
    if (!canvas)
      return;
    lv_area_t area;
    area.x1 = (int32_t) x0;
    area.y1 = (int32_t) y0;
    area.x2 = (int32_t) (x0 + w - 1);
    area.y2 = (int32_t) (y0 + h - 1);
    lv_obj_invalidate_area(canvas, &area);
  }
};

}  // namespace fxutil
}  // namespace lvgl_canvas_fx
}  // namespace esphome
