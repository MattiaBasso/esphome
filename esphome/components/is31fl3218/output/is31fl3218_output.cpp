#include "is31fl3218_output.h"

namespace esphome {
namespace is31fl3218 {

void IS31FL3218Channel::write_state(float state) {
  auto amount = static_cast<uint16_t>(state * 0xfff);
  this->parent_->set_channel_value(this->channel_, amount);
}

}  // namespace is31fl3218
}  // namespace esphome
