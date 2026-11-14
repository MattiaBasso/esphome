#pragma once
// Helper per gli effetti dell'anello LED (partizione voice_assistant_leds, che ha gamma_correct: 1.0).
//
// Con la gamma di ESPHome ogni canale passa da una tabella che porta a 1 qualunque valore non nullo, e a bassa
// luminosità i canali arrivano a zero in momenti diversi: un arancio che sfuma diventa blu (il 5% di blu resta a
// 1 mentre il rosso scende a 2-3) e poi rosso (il verde va a zero prima). Qui la gamma si applica una volta sola
// al colore intero: i tre canali scalano insieme e la tinta resta quella scelta. Quando un canale importante non
// si può più rappresentare, il LED si spegne invece di cambiare colore.

#include <algorithm>
#include <cmath>

#include "esphome/components/light/light_state.h"
#include "esphome/core/color.h"

namespace vimar_leds {

using esphome::Color;

static constexpr float GAMMA = 2.8f;  // come il default delle light di ESPHome

/// Colore della light (canali 0-255, senza luminosità né gamma)
inline Color base_color(esphome::light::LightState *light) {
  const auto &v = light->current_values;
  return Color(static_cast<uint8_t>(v.get_red() * 255), static_cast<uint8_t>(v.get_green() * 255),
               static_cast<uint8_t>(v.get_blue() * 255));
}

/// `base` all'intensità k (0-1, percettiva) e alla luminosità `brightness` (0-1), già con la gamma
inline Color dim(const Color &base, float k, float brightness) {
  const float level = powf(std::clamp(k * brightness, 0.0f, 1.0f), GAMMA);
  const uint8_t in[3] = {base.r, base.g, base.b};
  const uint8_t max_in = std::max({in[0], in[1], in[2]});
  if (max_in == 0 || level <= 0.0f)
    return Color::BLACK;
  uint8_t out[3];
  for (int c = 0; c < 3; c++) {
    const long v = lroundf(powf(in[c] / 255.0f, GAMMA) * level * 255.0f);
    out[c] = static_cast<uint8_t>(std::min(v, 255L));
    // canale importante (almeno 1/4 del massimo) sparito: la tinta non regge più
    if (out[c] == 0 && in[c] * 4 >= max_in)
      return Color::BLACK;
  }
  return Color(out[0], out[1], out[2]);
}

inline bool is_black(const Color &c) { return (c.r | c.g | c.b) == 0; }

/// Intensità minima (0-1) a cui `base` si vede ancora con la sua tinta, alla luminosità `brightness`
inline float min_visible(const Color &base, float brightness) {
  if (is_black(dim(base, 1.0f, brightness)))
    return 1.0f;
  float lo = 0.0f, hi = 1.0f;
  for (int i = 0; i < 12; i++) {
    const float mid = 0.5f * (lo + hi);
    if (is_black(dim(base, mid, brightness))) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  return hi;
}

/// Come dim(), ma f (0-1) parte dal minimo visibile: f > 0 accende sempre il LED, a passi uguali fino a 1
inline Color dim_visible(const Color &base, float f, float brightness) {
  if (f <= 0.01f)
    return Color::BLACK;
  const float k_min = min_visible(base, brightness);
  return dim(base, k_min + (1.0f - k_min) * std::min(f, 1.0f), brightness);
}

}  // namespace vimar_leds
