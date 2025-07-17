#include "is31fl3218_output.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace is31fl3218 {

static const char *const TAG = "is31fl3218";

// --- IS31FL3218 Register Map ---
const uint8_t IS31FL3218_REG_SHUTDOWN = 0x00;
const uint8_t IS31FL3218_REG_CONTROL_1 = 0x13;
const uint8_t IS31FL3218_REG_CONTROL_2 = 0x14;
const uint8_t IS31FL3218_REG_CONTROL_3 = 0x15;
const uint8_t IS31FL3218_REG_UPDATE = 0x16;
const uint8_t IS31FL3218_REG_RESET = 0x17;

// --- Control Register Values ---
const uint8_t IS31FL3218_SHUTDOWN_MODE = 0x00;
const uint8_t IS31FL3218_NORMAL_MODE = 0x01;
const uint8_t IS31FL3218_RESET_COMMAND = 0x00;
const uint8_t IS31FL3218_UPDATE_COMMAND = 0x00;

// --- Channel Control Register Values ---
const uint8_t IS31FL3218_CHANNEL_REGISTER_ON = 0x3F;
const uint8_t IS31FL3218_CHANNEL_OFF = 0x00;
const uint8_t IS31FL3218_CHANNEL_ON = 0x01;

void IS31FL3218Output::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  // Software reset
  if (!this->write_byte(IS31FL3218_REG_RESET, IS31FL3218_RESET_COMMAND)) {
    ESP_LOGE(TAG, "Reset failed");
    this->mark_failed();
    return;
  }
  
  // Set to normal operation mode
  if (!this->write_byte(IS31FL3218_REG_SHUTDOWN, IS31FL3218_NORMAL_MODE)) {
    ESP_LOGE(TAG, "Failed to set normal mode");
    this->mark_failed();
    return;
  }

  // Enable all channels in the control register 1
  if (!this->write_bytes(IS31FL3218_REG_CONTROL_1, IS31FL3218_CHANNEL_REGISTER_ON)) {
    ESP_LOGE(TAG, "Failed to set control register 1");
    this->mark_failed();
    return;
  }

  // Enable all channels in the control register 2
  if (!this->write_bytes(IS31FL3218_REG_CONTROL_2, IS31FL3218_CHANNEL_REGISTER_ON)) {
    ESP_LOGE(TAG, "Failed to set control register 1");
    this->mark_failed();
    return;
  }

  // Enable all channels in the control register 3
  if (!this->write_bytes(IS31FL3218_REG_CONTROL_3, IS31FL3218_CHANNEL_REGISTER_ON)) {
    ESP_LOGE(TAG, "Failed to set control register 1");
    this->mark_failed();
    return;
  }

  // Initialize all PWM values to 0
  for (uint8_t i = 0; i < 18; i++) {
    if (!this->write_byte(0x01 + i, 0)) {
      ESP_LOGE(TAG, "Failed to initialize channel %d", i);
      this->mark_failed();
      return;
    }
  }

  // Update the register to apply changes
  if (!this->write_byte(IS31FL3218_REG_UPDATE, IS31FL3218_UPDATE_COMMAND)) {
    ESP_LOGE(TAG, "Failed to update register");
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "IS31FL3218 initialized successfully");
  
  //this->loop();
}

void IS31FL3218Output::dump_config()
{
  ESP_LOGCONFIG(TAG, "IS31FL3218:");
  LOG_I2C_DEVICE(this);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Setting up IS31FL3218 failed!");
  }
}

void IS31FL3218Output::loop() {
  if (this->min_channel_ == 0xFF || !this->update_)
    return;

  // Write PWM values for all changed channels
  for (uint8_t channel = this->min_channel_; channel <= this->max_channel_; channel++) {
    uint8_t pwm = this->pwm_amounts_[channel];
    ESP_LOGVV(TAG, "Setting channel %d to PWM value %d", channel, pwm);

    if (!this->write_byte(0x01 + channel, pwm)) {
      ESP_LOGE(TAG, "Failed to write PWM value for channel %d", channel);
      this->status_set_warning();
      return;
    }
  }

  // Update the register to apply all changes
  if (!this->write_byte(IS31FL3218_REG_UPDATE, IS31FL3218_UPDATE_COMMAND)) {
    ESP_LOGE(TAG, "Failed to update register");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  this->update_ = false;

}

void IS31FL3218Output::register_channel(IS31FL3218Channel *channel) {
  auto c = channel->channel_;
  if (c >= 18) {
    ESP_LOGE(TAG, "Channel %d is out of range (0-17)", c);
    return;
  }

  this->min_channel_ = std::min(this->min_channel_, c);
  this->max_channel_ = std::max(this->max_channel_, c);
  channel->set_parent(this);
}

void IS31FL3218Channel::write_state(float state) {
  if (this->channel_ >= 18) {
    ESP_LOGE(TAG, "Channel %d is out of range", this->channel_);
    return;
  }

  const uint8_t max_duty = 255;
  const float duty_rounded = roundf(state * max_duty);
  auto duty = static_cast<uint8_t>(duty_rounded);

  ESP_LOGVV(TAG, "Channel %d: state=%.3f, duty=%d", this->channel_, state, duty);
  this->parent_->set_channel_value_(this->channel_, duty);
}

}  // namespace is31fl3218
}  // namespace esphome