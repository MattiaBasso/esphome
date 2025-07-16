#pragma once

#include "esphome/core/helpers.h"

#include "esphome/components/output/float_output.h"

#include "../is31fl3218.h"

namespace esphome {
namespace is31fl3218 {

class IS31FL3218Channel : public output::FloatOutput, public Parented<IS31FL3218> {
 public:
  void set_channel(uint8_t channel) { this->channel_ = channel; }

 protected:
  void write_state(float state) override;
  uint8_t channel_;
};

}  // namespace is31fl3218
}  // namespace esphome
