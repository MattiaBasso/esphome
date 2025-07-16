#include "is31fl3218.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace is31fl3218 {

static const char *const TAG = "is31fl3218";

void IS31FL3218Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up IS31FL3218...");
  
  // Reset the device
  if (!this->write_register(REG_RESET, 0x00)) {
    ESP_LOGE(TAG, "Failed to reset IS31FL3218");
    this->mark_failed();
    return;
  }
  
  delay(10);  // Wait for reset to complete
  
  // Exit shutdown mode
  if (!this->write_register(REG_SHUTDOWN, 0x01)) {
    ESP_LOGE(TAG, "Failed to exit shutdown mode");
    this->mark_failed();
    return;
  }
  
  // Enable all channels in control registers
  // Control register 1: channels 1-6
  if (!this->write_register(REG_CONTROL_BASE, 0x3F)) {
    ESP_LOGE(TAG, "Failed to configure control register 1");
    this->mark_failed();
    return;
  }
  
  // Control register 2: channels 7-12
  if (!this->write_register(REG_CONTROL_BASE + 1, 0x3F)) {
    ESP_LOGE(TAG, "Failed to configure control register 2");
    this->mark_failed();
    return;
  }
  
  // Control register 3: channels 13-18
  if (!this->write_register(REG_CONTROL_BASE + 2, 0x3F)) {
    ESP_LOGE(TAG, "Failed to configure control register 3");
    this->mark_failed();
    return;
  }
  
  // Initialize all PWM registers to 0
  for (uint8_t i = 0; i < 18; i++) {
    if (!this->write_register(REG_PWM_BASE + i, 0x00)) {
      ESP_LOGE(TAG, "Failed to initialize PWM register %d", i);
      this->mark_failed();
      return;
    }
  }
  
  // Update the outputs
  this->update_outputs();
  
  ESP_LOGCONFIG(TAG, "IS31FL3218 initialized successfully");
}

void IS31FL3218Component::dump_config() {
  ESP_LOGCONFIG(TAG, "IS31FL3218:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with IS31FL3218 failed!");
  }
}

void IS31FL3218Component::set_channel_value(uint8_t channel, float value) {
  if (channel >= 18) {
    ESP_LOGE(TAG, "Invalid channel: %d", channel);
    return;
  }
  
  // Clamp value between 0.0 and 1.0
  value = clamp(value, 0.0f, 1.0f);
  
  if (this->channel_values_[channel] != value) {
    this->channel_values_[channel] = value;
    this->needs_update_ = true;
    
    // Convert float to 8-bit PWM value
    uint8_t pwm_value = (uint8_t)(value * 255);
    
    // Write to PWM register
    if (!this->write_register(REG_PWM_BASE + channel, pwm_value)) {
      ESP_LOGE(TAG, "Failed to set PWM value for channel %d", channel);
    }
  }
}

void IS31FL3218Component::update_outputs() {
  if (this->needs_update_) {
    // Trigger update by writing to update register
    if (!this->write_register(REG_UPDATE, 0x00)) {
      ESP_LOGE(TAG, "Failed to update outputs");
    } else {
      this->needs_update_ = false;
    }
  }
}

bool IS31FL3218Component::write_register(uint8_t reg, uint8_t value) {
  return this->write_byte(reg, value);
}

bool IS31FL3218Component::read_register(uint8_t reg, uint8_t &value) {
  return this->read_byte(reg, &value);
}

// IS31FL3218Channel implementation
void IS31FL3218Channel::write_state(float state) {
  this->parent_->set_channel_value(this->channel_, state);
  this->parent_->update_outputs();
}

}  // namespace is31fl3218
}  // namespace esphome