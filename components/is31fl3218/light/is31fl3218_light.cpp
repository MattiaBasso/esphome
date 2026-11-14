#include "is31fl3218_light.h"

#include "esphome/core/log.h"

namespace esphome::is31fl3218 {

static const char *const TAG = "is31fl3218.light";

void IS31FL3218Light::add_led(uint8_t red, uint8_t green, uint8_t blue) {
  if (this->num_leds_ >= MAX_LEDS)
    return;
  this->channels_[this->num_leds_][0] = red;
  this->channels_[this->num_leds_][1] = green;
  this->channels_[this->num_leds_][2] = blue;
  this->num_leds_++;
}

light::LightTraits IS31FL3218Light::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void IS31FL3218Light::write_state(light::LightState *state) {
  for (uint8_t i = 0; i < this->num_leds_; i++) {
    for (uint8_t c = 0; c < 3; c++)
      this->parent_->set_channel_value(this->channels_[i][c], this->buf_[i][c]);
  }
  this->mark_shown_();
}

void IS31FL3218Light::clear_effect_data() {
  for (auto &d : this->effect_data_)
    d = 0;
}

light::ESPColorView IS31FL3218Light::get_view_internal(int32_t index) const {
  uint8_t *px = this->buf_[index];
  return {px + 0, px + 1, px + 2, nullptr, &this->effect_data_[index], &this->correction_};
}

void IS31FL3218Light::dump_config() {
  ESP_LOGCONFIG(TAG, "IS31FL3218 light: %u LED", this->num_leds_);
  for (uint8_t i = 0; i < this->num_leds_; i++) {
    ESP_LOGCONFIG(TAG, "  LED %u: R=OUT%u G=OUT%u B=OUT%u", i, this->channels_[i][0] + 1, this->channels_[i][1] + 1,
                  this->channels_[i][2] + 1);
  }
}

}  // namespace esphome::is31fl3218

