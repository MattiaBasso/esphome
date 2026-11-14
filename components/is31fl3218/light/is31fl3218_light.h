#pragma once

#include "esphome/components/light/addressable_light.h"
#include "../is31fl3218.h"

namespace esphome::is31fl3218 {

/// Addressable RGB light on top of the IS31FL3218: pixel n = three driver channels.
/// write_state() only stores PWM values; the hub writes the changed channels in its next loop().
class IS31FL3218Light final : public light::AddressableLight, public Parented<IS31FL3218> {
 public:
  static constexpr uint8_t MAX_LEDS = IS31FL3218_NUM_CHANNELS / 3;

  void add_led(uint8_t red, uint8_t green, uint8_t blue);

  int32_t size() const override { return this->num_leds_; }
  light::LightTraits get_traits() override;
  void write_state(light::LightState *state) override;
  void clear_effect_data() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  light::ESPColorView get_view_internal(int32_t index) const override;

  uint8_t num_leds_{0};
  uint8_t channels_[MAX_LEDS][3]{};      // R, G, B channel of each pixel
  mutable uint8_t buf_[MAX_LEDS][3]{};   // R, G, B value (already gamma corrected)
  mutable uint8_t effect_data_[MAX_LEDS]{};
};

}  // namespace esphome::is31fl3218

