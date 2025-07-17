#include "esphome/core/log.h"
#include "is31fl3218.h"

namespace esphome {
namespace is31fl3218 {

static const char *TAG = "is31fl3218.output";

void IS31FL3218::setup(){

}

void IS31FL3218::write_state(float state){

}

void IS31FL3218::dump_config() {
    ESP_LOGCONFIG(TAG, "IS31FL3218 output");
}

} //namespace is31fl3218
} //namespace esphome
