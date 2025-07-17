#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace is31fl3218 {

class IS31FL3218Output;

class IS31FL3218Channel : public output::FloatOutput, public Parented<IS31FL3218Output> {
 public:
  void set_channel(uint8_t channel) { channel_ = channel; }

 protected:
  friend class IS31FL3218Output;

  void write_state(float state) override;

  uint8_t channel_;
};

/// IS31FL3218 float output component.
class IS31FL3218Output : public Component, public i2c::I2CDevice {
 public:
  IS31FL3218Output() = default;

  void register_channel(IS31FL3218Channel *channel);

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void loop() override;

 protected:
  friend IS31FL3218Channel;

  void set_channel_value_(uint8_t channel, uint8_t value) {
    if (this->pwm_amounts_[channel] != value)
      this->update_ = true;
    this->pwm_amounts_[channel] = value;
  }

  uint8_t min_channel_{0xFF};
  uint8_t max_channel_{0x00};
  uint8_t pwm_amounts_[18] = {0};
  bool update_{true};
};

}  // namespace is31fl3218
}  // namespace esphome