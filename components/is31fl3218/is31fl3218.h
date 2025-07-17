#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace is31fl3218 {

class IS31FL3218Component : public i2c::I2CDevice, public Component {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;
};

class IS31FL3218Output : public output::FloatOutput, public Component {
 public:
  void setup() override;
  void write_state(float state) override;
  void dump_config() override;
};

}  // namespace is31fl3218
}  // namespace esphome
