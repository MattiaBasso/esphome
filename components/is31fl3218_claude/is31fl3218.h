#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"

namespace esphome {
namespace is31fl3218 {

static const char *const TAG = "is31fl3218";

// Register addresses
static const uint8_t IS31FL3218_REG_SHUTDOWN = 0x00;
static const uint8_t IS31FL3218_REG_PWM_START = 0x01;
static const uint8_t IS31FL3218_REG_CTRL_START = 0x13;
static const uint8_t IS31FL3218_REG_UPDATE = 0x16;
static const uint8_t IS31FL3218_REG_RESET = 0x17;

// Constants
static const uint8_t IS31FL3218_NUM_CHANNELS = 18;
static const uint8_t IS31FL3218_DEFAULT_ADDRESS = 0x54;

// Gamma correction lookup table
static const uint8_t GAMMA_CORRECTION[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 20,
    22, 24, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 57,
    61, 65, 69, 73, 77, 81, 85, 89, 94, 99, 104, 109,
    114, 119, 124, 129, 134, 140, 146, 152, 158, 164,
    170, 176, 182, 188, 195, 202, 209, 216, 223, 230,
    237, 244, 251, 255
};

class IS31FL3218Component : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  // Set channel brightness (0.0 to 1.0)
  void set_channel(uint8_t channel, float brightness);
  
  // Set multiple channels at once
  void set_channels(const std::vector<float> &brightnesses);
  
  // Enable/disable channels
  void set_channel_enabled(uint8_t channel, bool enabled);
  void set_all_channels_enabled(bool enabled);
  
  // Update all channels
  void update_channels();
  
  // Shutdown control
  void set_shutdown(bool shutdown);

 protected:
  bool write_register(uint8_t reg, uint8_t value);
  bool write_registers(uint8_t start_reg, const uint8_t *data, size_t len);
  uint8_t gamma_correct(float brightness);
  
  bool enabled_channels_[IS31FL3218_NUM_CHANNELS];
  uint8_t pwm_values_[IS31FL3218_NUM_CHANNELS];
  bool needs_update_{false};
};

// Individual channel light output
class IS31FL3218Channel : public light::LightOutput {
 public:
  IS31FL3218Channel(IS31FL3218Component *parent, uint8_t channel) 
      : parent_(parent), channel_(channel) {}

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(light::LightState *state) override {
    float brightness;
    state->current_values_as_brightness(&brightness);
    parent_->set_channel(channel_, brightness);
  }

 protected:
  IS31FL3218Component *parent_;
  uint8_t channel_;
};

}  // namespace is31fl3218
}  // namespace esphome