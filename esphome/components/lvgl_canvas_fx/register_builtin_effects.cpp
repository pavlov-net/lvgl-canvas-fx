// © Copyright 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

#include "register_builtin_effects.h"
#include "fx_registry.h"

// include each effect's header
#include "fx_circle.h"
#include "fx_fireplace.h"
#include "fx_fireworks_physics.h"
#include "fx_aurora.h"
#include "fx_audio_spectrum.h"
#include "fx_plasma.h"
#include "fx_breathing.h"
#include "fx_clouds.h"
#include "fx_sunset.h"
#include "fx_starfield.h"
#include "fx_snowfall.h"
#include "fx_leaves.h"
#include "fx_fireflies.h"
#include "fx_warp.h"
#include "fx_boids.h"
#include "fx_ripples.h"
#include "fx_matrix.h"
#include "fx_sand.h"
#include "fx_lightning.h"
#include "fx_lava_lamp.h"
#include "fx_rain.h"

namespace esphome {
namespace lvgl_canvas_fx {

void register_builtin_effects() {
  // One line per effect:
  FxRegistry::register_factory("circle", [] { return std::make_unique<FxCircle>(); });

  FxRegistry::register_factory("fireplace", [] { return std::make_unique<FxFireplace>(); });

  FxRegistry::register_factory("fireworks", [] { return std::make_unique<FxFireworksPhysics>(); });

  FxRegistry::register_factory("aurora", [] {
    auto fx = std::make_unique<FxAurora>();
    // Optional: tune defaults here if you want different look/speed
    // fx->set_speed(0.05f);
    // fx->set_scale(24);
    // fx->set_intensity(192);
    return fx;
  });

  FxRegistry::register_factory("audio_spectrum", [] {
    auto fx = std::make_unique<FxAudioSpectrum>();

    // Music friendly settings
    // 512, 8, 16000.0, 0.022f, 0.25f, 9.0f, -55.0f, 100.0f, 0.47f
    fx->configure(512, 8, 16000.0f, 0.022f, 0.25f, 9.0f, -55.0f, 100.0f, 0.47f);

    fx->set_colors(lv_color_hex(0x00FFD0), lv_color_hex(0xFF4000));
    fx->set_gap_px(1);
    fx->set_round_to_mult8(true);

    return fx;
  });

  FxRegistry::register_factory("plasma", [] { return std::make_unique<FxPlasma>(); });

  FxRegistry::register_factory("breathing", [] { return std::make_unique<FxBreathing>(); });

  FxRegistry::register_factory("clouds", [] { return std::make_unique<FxClouds>(); });

  FxRegistry::register_factory("sunset", [] { return std::make_unique<FxSunset>(); });

  FxRegistry::register_factory("starfield", [] { return std::make_unique<FxStarfield>(); });

  FxRegistry::register_factory("snowfall", [] { return std::make_unique<FxSnowfall>(); });

  FxRegistry::register_factory("leaves", [] {
    auto fx = std::make_unique<FxLeaves>();
    fx->set_variant(FxLeaves::Variant::AUTUMN);
    return fx;
  });

  FxRegistry::register_factory("sakura", [] {
    auto fx = std::make_unique<FxLeaves>();
    fx->set_variant(FxLeaves::Variant::SAKURA);
    return fx;
  });

  FxRegistry::register_factory("fireflies", [] { return std::make_unique<FxFireflies>(); });

  FxRegistry::register_factory("warp", [] { return std::make_unique<FxWarp>(); });

  FxRegistry::register_factory("boids", [] { return std::make_unique<FxBoids>(); });

  FxRegistry::register_factory("ripples", [] { return std::make_unique<FxRipples>(); });

  FxRegistry::register_factory("matrix", [] { return std::make_unique<FxMatrix>(); });

  FxRegistry::register_factory("sand", [] { return std::make_unique<FxSand>(); });

  FxRegistry::register_factory("lightning", [] { return std::make_unique<FxLightning>(); });

  FxRegistry::register_factory("lava_lamp", [] { return std::make_unique<FxLavaLamp>(); });

  FxRegistry::register_factory("rain", [] {
    auto fx = std::make_unique<FxRain>();
    fx->set_intensity(0.5f);  // default until submit_data() provides a value
    return fx;
  });
}

}  // namespace lvgl_canvas_fx
}  // namespace esphome
