#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace is31fl3218 {

class IS31FL3218 : public output::FloatOutput, public Component {
 public:
  void setup() override;
  void write_state(float state) override;
  void dump_config() override;
};


} //namespace is31fl3218
} //namespace esphome