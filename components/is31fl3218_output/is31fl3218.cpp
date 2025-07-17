#include "esphome/core/log.h"
#include "is31fl3218.h"

namespace esphome {
namespace is31fl3218 {

static const char *TAG = "is31fl3218.output";

void IS31FL3218::setup(){
    ESP_LOGCONFIG(TAG, "Setting up IS31FL3218...");
    if (this->write(&RESET_REG, 0x00) != i2c::ERROR_OK) {
        this->mark_failed(); // Mark the component as failed if communication fails
        return;
    }

    if (this->write(&SHUTDOWN_REG, 0x01) != i2c::ERROR_OK) {
        this->mark_failed(); // Mark the component as failed if communication fails
        return;
    }

    if ((this->write(&LED_CTRL_REG_1, &LDR_ON) != i2c::ERROR_OK) && 
       (this->write(&LED_CTRL_REG_2, &LDR_ON) != i2c::ERROR_OK) && 
       (this->write(&LED_CTRL_REG_3, &LDR_ON) != i2c::ERROR_OK)){
        this->mark_failed(); // Mark the component as failed if communication fails
        return;
    }    
    ESP_LOGCONFIG(TAG, "Done setting up IS31FL3218 component.");
}

void IS31FL3218::write_state(float state){

}

void IS31FL3218::dump_config() {
    ESP_LOGCONFIG(TAG, "IS31FL3218 output");
}

} //namespace is31fl3218
} //namespace esphome
