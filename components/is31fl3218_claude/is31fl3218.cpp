// is31fl3218.cpp
#include "is31fl3218.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace is31fl3218 {

void IS31FL3218Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up IS31FL3218...");
  
  // Initialize arrays
  for (uint8_t i = 0; i < IS31FL3218_NUM_CHANNELS; i++) {
    enabled_channels_[i] = true;
    pwm_values_[i] = 0;
  }
  
  // Reset the device
  if (!this->write_register(IS31FL3218_REG_RESET, 0x00)) {
    ESP_LOGE(TAG, "Failed to reset device");
    this->mark_failed();
    return;
  }
  
  delay(10);  // Wait for reset
  
  // Disable shutdown (enable device)
  if (!this->write_register(IS31FL3218_REG_SHUTDOWN, 0xFF)) {
    ESP_LOGE(TAG, "Failed to enable device");
    this->mark_failed();
    return;
  }
  
  // Enable all channels by default
  this->set_all_channels_enabled(true);
  
  // Clear all PWM values
  uint8_t zero_data[IS31FL3218_NUM_CHANNELS] = {0};
  if (!this->write_registers(IS31FL3218_REG_PWM_START, zero_data, IS31FL3218_NUM_CHANNELS)) {
    ESP_LOGE(TAG, "Failed to clear PWM registers");
    this->mark_failed();
    return;
  }
  
  // Update registers
  this->update_channels();
  
  ESP_LOGCONFIG(TAG, "IS31FL3218 setup complete");
}

void IS31FL3218Component::dump_config() {
  ESP_LOGCONFIG(TAG, "IS31FL3218:");
  LOG_I2C_DEVICE(this);
  
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with IS31FL3218 failed!");
  }
}

void IS31FL3218Component::set_channel(uint8_t channel, float brightness) {
  if (channel >= IS31FL3218_NUM_CHANNELS) {
    ESP_LOGE(TAG, "Invalid channel: %d", channel);
    return;
  }
  
  // Clamp brightness to valid range
  brightness = clamp(brightness, 0.0f, 1.0f);
  
  // Apply gamma correction
  uint8_t pwm_value = this->gamma_correct(brightness);
  
  if (pwm_values_[channel] != pwm_value) {
    pwm_values_[channel] = pwm_value;
    
    // Write to PWM register
    if (!this->write_register(IS31FL3218_REG_PWM_START + channel, pwm_value)) {
      ESP_LOGE(TAG, "Failed to write PWM value for channel %d", channel);
      return;
    }
    
    // Trigger update
    this->update_channels();
  }
}

void IS31FL3218Component::set_channels(const std::vector<float> &brightnesses) {
  size_t num_channels = std::min(brightnesses.size(), static_cast<size_t>(IS31FL3218_NUM_CHANNELS));
  
  uint8_t pwm_data[IS31FL3218_NUM_CHANNELS];
  bool changed = false;
  
  for (size_t i = 0; i < num_channels; i++) {
    float brightness = clamp(brightnesses[i], 0.0f, 1.0f);
    uint8_t pwm_value = this->gamma_correct(brightness);
    pwm_data[i] = pwm_value;
    
    if (pwm_values_[i] != pwm_value) {
      pwm_values_[i] = pwm_value;
      changed = true;
    }
  }
  
  if (changed) {
    if (!this->write_registers(IS31FL3218_REG_PWM_START, pwm_data, num_channels)) {
      ESP_LOGE(TAG, "Failed to write PWM values");
      return;
    }
    this->update_channels();
  }
}

void IS31FL3218Component::set_channel_enabled(uint8_t channel, bool enabled) {
  if (channel >= IS31FL3218_NUM_CHANNELS) {
    ESP_LOGE(TAG, "Invalid channel: %d", channel);
    return;
  }
  
  if (enabled_channels_[channel] != enabled) {
    enabled_channels_[channel] = enabled;
    
    // Calculate control register and bit position
    uint8_t ctrl_reg = channel / 6;  // 0, 1, or 2
    uint8_t bit_pos = channel % 6;
    
    // Read current control register value
    uint8_t ctrl_value = 0;
    
    // Build control value for this register (6 channels per register)
    for (uint8_t i = 0; i < 6; i++) {
      uint8_t ch = ctrl_reg * 6 + i;
      if (ch < IS31FL3218_NUM_CHANNELS && enabled_channels_[ch]) {
        ctrl_value |= (1 << i);
      }
    }
    
    // Write control register
    if (!this->write_register(IS31FL3218_REG_CTRL_START + ctrl_reg, ctrl_value)) {
      ESP_LOGE(TAG, "Failed to write control register %d", ctrl_reg);
    }
  }
}

void IS31FL3218Component::set_all_channels_enabled(bool enabled) {
  for (uint8_t i = 0; i < IS31FL3218_NUM_CHANNELS; i++) {
    enabled_channels_[i] = enabled;
  }
  
  // Write all control registers
  uint8_t ctrl_value = enabled ? 0x3F : 0x00;  // 0x3F = 0b00111111 (6 bits)
  uint8_t ctrl_data[3] = {ctrl_value, ctrl_value, ctrl_value};
  
  if (!this->write_registers(IS31FL3218_REG_CTRL_START, ctrl_data, 3)) {
    ESP_LOGE(TAG, "Failed to write control registers");
  }
}

void IS31FL3218Component::update_channels() {
  if (!this->write_register(IS31FL3218_REG_UPDATE, 0x01)) {
    ESP_LOGE(TAG, "Failed to update channels");
  }
}

void IS31FL3218Component::set_shutdown(bool shutdown) {
  uint8_t value = shutdown ? 0x00 : 0xFF;
  if (!this->write_register(IS31FL3218_REG_SHUTDOWN, value)) {
    ESP_LOGE(TAG, "Failed to set shutdown mode");
  }
}

bool IS31FL3218Component::write_register(uint8_t reg, uint8_t value) {
  return this->write_byte(reg, value);
}

bool IS31FL3218Component::write_registers(uint8_t start_reg, const uint8_t *data, size_t len) {
  return this->write_bytes(start_reg, data, len);
}

uint8_t IS31FL3218Component::gamma_correct(float brightness) {
  if (brightness <= 0.0f) return 0;
  if (brightness >= 1.0f) return 255;
  
  // Scale to gamma table index (0-63)
  uint8_t index = static_cast<uint8_t>(brightness * 63.0f);
  return GAMMA_CORRECTION[index];
}

}  // namespace is31fl3218
}  // namespace esphome