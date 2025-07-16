#include "esphome/core/log.h"
#include "is31fl3218.h"

namespace esphome {
namespace is31fl3218 {

const uint8_t SHUTDOWN_REG = 0x00;
const uint8_t UPDATE_REG = 0x16;
const uint8_t RESET_REG = 0x17;

static const char *TAG = "is31fl3218.component";

void IS31FL3218::setup() {
    // I2C device initialization is typically done here.
    // Note that a number of read/write methods are available in the I2CDevice
    // class. See "i2c/i2c.h" for details.
    ESP_LOGCONFIG(TAG, "Setting up IS31FL3218...");
    if (this->write(0x00, RESET_REG) != i2c::ERROR_OK) {
    this->mark_failed(); // Mark the component as failed if communication fails
    return;
    }
    
    this->pwm_amounts_.resize(this->num_chips_ * N_CHANNELS_PER_CHIP, 0);
    
    ESP_LOGCONFIG(TAG, "Done setting up IS31FL3218 component.");
}

void IS31FL3218::loop() {

}

void IS31FL3218::dump_config(){
    ESP_LOGCONFIG(TAG, "IS31FL3218");
    LOG_I2C_DEVICE(this)
    if (this->is_failed()) {
        ESP_LOGE(TAG, "Communication with IS31FL3218 failed!");
    }
}

void IS31FL3218::set_channel_value(uint16_t channel, uint16_t value) {
  if (channel >= this->num_chips_ * N_CHANNELS_PER_CHIP)
    return;
  if (this->pwm_amounts_[channel] != value) {
    this->update_ = true;
  }
  this->pwm_amounts_[channel] = value;
}

}  // namespace is31fl3218
}  // namespace esphome
