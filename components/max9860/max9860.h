#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace max9860 {

class MAX9860Component : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Function called by python to set the variable
  void set_startup_volume(uint8_t volume) { this->startup_volume_ = volume; }

 protected:
  uint8_t startup_volume_{0x06}; // Default internal value
};

}  // namespace max9860
}  // namespace esphome