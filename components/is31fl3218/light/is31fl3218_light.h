#pragma once

#include "esphome/components/light/addressable_light.h"
#include "../is31fl3218.h"

namespace esphome::is31fl3218 {

/// Addressable RGB light on top of the IS31FL3218: pixel n = three driver channels.
/// write_state() only stores PWM values; the hub writes the changed channels in its next loop().
class IS31FL3218Light final : public light::AddressableLight, public Parented<IS31FL3218> {
 public:
  static constexpr uint8_t MAX_LEDS = IS31FL3218_NUM_CHANNELS / 3;

  IS31FL3218Light() { this->accessor_ = &this->color_accessor_; }

  void add_led(uint8_t red, uint8_t green, uint8_t blue);

  int32_t size() const override { return this->num_leds_; }
  light::LightTraits get_traits() override;
  void write_state(light::LightState *state) override;
  void clear_effect_data() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  uint8_t num_leds_{0};
  uint8_t channels_[MAX_LEDS][3]{};  // R, G, B channel of each pixel
  uint8_t buf_[MAX_LEDS][3]{};       // R, G, B value (already gamma corrected)
  uint8_t effect_data_[MAX_LEDS]{};
  // Pixel access for AddressableLight: corrects on write, uncorrects on read
  light::CorrectedBufferColorAccessor color_accessor_{&this->buf_[0][0], this->effect_data_,
                                                      light::ChannelColors(0, 1, 2), &this->correction_};
};

}  // namespace esphome::is31fl3218

