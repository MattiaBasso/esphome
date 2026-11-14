#include "is31fl3218.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome::is31fl3218 {

static const char *const TAG = "is31fl3218";

// Mark the component as failed; use only in setup
#define IS31FL3218_ERROR_FAILED(func) \
  if (!(func)) { \
    this->mark_failed(); \
    return; \
  }

void IS31FL3218::setup() {
  if (this->enable_pin_ != nullptr) {
    this->enable_pin_->setup();
    this->enable_pin_->digital_write(true);  // SDB high = enabled
  }

  IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_RESET, 0x00));
  IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_SHUTDOWN, IS31FL3218_NORMAL_OPERATION));
  IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_LED_CTRL_1, IS31FL3218_LED_CTRL_ALL_ON));
  IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_LED_CTRL_2, IS31FL3218_LED_CTRL_ALL_ON));
  IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_LED_CTRL_3, IS31FL3218_LED_CTRL_ALL_ON));
  for (uint8_t ch = 0; ch < IS31FL3218_NUM_CHANNELS; ch++) {
    IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_PWM_BASE + ch, this->pwm_[ch]));
  }
  IS31FL3218_ERROR_FAILED(this->write_byte(IS31FL3218_REG_UPDATE, 0x00));

  this->dirty_mask_ = 0;
  this->disable_loop();  // re-enabled only when a channel changes
}

void IS31FL3218::dump_config() {
  ESP_LOGCONFIG(TAG, "IS31FL3218 LED driver:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Enable Pin: ", this->enable_pin_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Failed to initialize!");
  }
}

void IS31FL3218::loop() {
  if (this->dirty_mask_ == 0) {
    this->disable_loop();
    return;
  }
  // One burst from the first to the last changed channel: the register address auto-increments (datasheet,
  // "Address auto increment"). Unchanged channels in between are rewritten with the same value.
  const uint8_t first = __builtin_ctz(this->dirty_mask_);
  const uint8_t last = 31 - __builtin_clz(this->dirty_mask_);
  if (!this->write_bytes(IS31FL3218_REG_PWM_BASE + first, this->pwm_ + first, last - first + 1)) {
    this->status_set_warning();
    return;  // retry on the next loop
  }
  if (!this->write_byte(IS31FL3218_REG_UPDATE, 0x00)) {
    this->status_set_warning();
    return;
  }
  this->status_clear_warning();
  this->dirty_mask_ = 0;
  this->disable_loop();
}

void IS31FL3218::register_channel(IS31FL3218Channel *channel) { channel->set_parent(this); }

void IS31FL3218::set_channel_value(uint8_t channel, uint8_t value) {
  if (channel >= IS31FL3218_NUM_CHANNELS || this->pwm_[channel] == value)
    return;
  this->pwm_[channel] = value;
  this->dirty_mask_ |= 1UL << channel;
  this->enable_loop();
}

void IS31FL3218Channel::write_state(float state) {
  const auto duty = static_cast<uint8_t>(std::lround(clamp(state, 0.0f, 1.0f) * 255.0f));
  this->parent_->set_channel_value(this->channel_, duty);
}

}  // namespace esphome::is31fl3218
