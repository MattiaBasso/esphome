#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace is31fl3218 {

class IS31FL3218Component : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  
  void set_channel_value(uint8_t channel, float value);
  void update_outputs();
  
 protected:
  bool write_register(uint8_t reg, uint8_t value);
  bool read_register(uint8_t reg, uint8_t &value);
  
  float channel_values_[18] = {0};
  bool needs_update_ = false;
  
  // IS31FL3218 Register addresses
  static const uint8_t REG_SHUTDOWN = 0x00;
  static const uint8_t REG_PWM_BASE = 0x01;  // PWM registers 0x01-0x12
  static const uint8_t REG_CONTROL_BASE = 0x13;  // Control registers 0x13-0x15
  static const uint8_t REG_UPDATE = 0x16;
  static const uint8_t REG_RESET = 0x17;
};

class IS31FL3218Channel : public output::FloatOutput {
 public:
  IS31FL3218Channel(IS31FL3218Component *parent, uint8_t channel) 
    : parent_(parent), channel_(channel) {}
  
  void write_state(float state) override;
  
 protected:
  IS31FL3218Component *parent_;
  uint8_t channel_;
};

}  // namespace is31fl3218
}  // namespace esphome