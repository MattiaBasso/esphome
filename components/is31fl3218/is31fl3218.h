#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace is31fl3218 {

class IS31FL3218 : public i2c::I2CDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_channel_value(uint16_t channel, uint16_t value);
 
protected:
  IS31FL3218Component *parent_;
  std::vector<uint16_t> pwm_amounts_;
};

}  // namespace is31fl3218
}  // namespace esphome
