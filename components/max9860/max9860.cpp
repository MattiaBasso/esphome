#include "max9860.h"
#include "esphome/core/log.h"

namespace esphome {
namespace max9860 {

static const char *const TAG = "max9860";

// Register Addresses
static const uint8_t REG_SYS_CLK = 0x03;
static const uint8_t REG_AUD_INT_A = 0x06; // Interface Control A
static const uint8_t REG_AUD_INT_B = 0x07; // Interface Control B
static const uint8_t REG_DAC_ATTEN = 0x09; // Output Volume
static const uint8_t REG_PWR_MAN = 0x10;   // Power Management

void MAX9860Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MAX9860...");

  // 1. Power Management: Shutdown bit (bit 7) = 1 to wake up.
  // We keep DAC disabled for now (bit 3 = 0).
  // 0x80 = 1000 0000
  this->write_byte(REG_PWR_MAN, 0x80);
  //delay(10); // Wait for wake up

  // 2. Interface Control A (0x06)
  // Bit 7 (Master/Slave): 0 = Slave (ESP32 is Master)
  // Bit 1-0 (Format): 10 = I2S Standard
  // Value: 0x02
  this->write_byte(REG_AUD_INT_A, 0x02);

  // 3. Interface Control B (0x07)
  // Settings for BCLK/LRCLK ratios. Default 0x00 is usually fine for standard I2S.
  this->write_byte(REG_AUD_INT_B, 0x00);

  // 4. DAC Attenuation (0x09)
  // 0x00 = 0dB (Max Volume)
  // 0x06 = -6dB
  // 0x0A = -10dB
  this->write_byte(REG_DAC_ATTEN, 0x06); // Set to -6dB

  // 5. System Clock (0x03)
  // Enable clock functionality if needed. 
  // 0x00 is default. If you use MCLK, configure here. 
  // For now, we rely on BCLK recovery or standard operation.
  this->write_byte(REG_SYS_CLK, 0x00);

  // 6. Enable DAC (0x10)
  // Bit 7 (SHDN) = 1, Bit 3 (DACEN) = 1
  // 0x88 = 1000 1000
  this->write_byte(REG_PWR_MAN, 0x88);

  ESP_LOGD(TAG, "MAX9860 Initialized");
}

void MAX9860Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MAX9860 Audio Codec:");
  LOG_I2C_DEVICE(this);
}

}  // namespace max9860
}  // namespace esphome