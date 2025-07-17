#include "esphome/core/log.h"
#include "is31fl3218.h"

namespace esphome {
namespace is31fl3218 {

const uint8_t SHUTDOWN_REG = 0x00;
const uint8_t UPDATE_REG = 0x16;
const uint8_t RESET_REG = 0x17;
const uint8_t LED_CTRL_REG_1 = 0x13;
const uint8_t LED_CTRL_REG_2 = 0x14;
const uint8_t LED_CTRL_REG_3 = 0x15;

static const uint8_t LDR_ON = 0x3F;

static const char *TAG = "is31fl3218.component";

void IS31FL3218::setup() {
    // I2C device initialization is typically done here.
    // Note that a number of read/write methods are available in the I2CDevice
    // class. See "i2c/i2c.h" for details.
    ESP_LOGCONFIG(TAG, "Setting up IS31FL3218...");
    if (this->write(RESET_REG, 0) != i2c::ERROR_OK) {
        this->mark_failed(); // Mark the component as failed if communication fails
        return;
    }

    if (this->write(SHUTDOWN_REG, 1) != i2c::ERROR_OK) {
        this->mark_failed(); // Mark the component as failed if communication fails
        return;
    }

    if ((this->write(LED_CTRL_REG_1, LDR_ON) != i2c::ERROR_OK) && 
       (this->write(LED_CTRL_REG_2, LDR_ON) != i2c::ERROR_OK) && 
       (this->write(LED_CTRL_REG_3, LDR_ON) != i2c::ERROR_OK)){
        this->mark_failed(); // Mark the component as failed if communication fails
        return;
    }    
    ESP_LOGCONFIG(TAG, "Done setting up IS31FL3218 component.");
}

void IS31FL3218::loop() {

}

void IS31FL3218::dump_config(){
    ESP_LOGCONFIG(TAG, "IS31FL3218");
}

}  // namespace is31fl3218
}  // namespace esphome
