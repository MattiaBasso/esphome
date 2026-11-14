#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

namespace esphome::is31fl3218 {

inline constexpr uint8_t IS31FL3218_NUM_CHANNELS = 18;

// Register map
inline constexpr uint8_t IS31FL3218_REG_SHUTDOWN = 0x00;  // 0x00 = software shutdown, 0x01 = normal
inline constexpr uint8_t IS31FL3218_REG_PWM_BASE = 0x01;  // 0x01..0x12: PWM of OUT1..OUT18
inline constexpr uint8_t IS31FL3218_REG_LED_CTRL_1 = 0x13;  // OUT1..6 enable, 6 bits each
inline constexpr uint8_t IS31FL3218_REG_LED_CTRL_2 = 0x14;  // OUT7..12
inline constexpr uint8_t IS31FL3218_REG_LED_CTRL_3 = 0x15;  // OUT13..18
inline constexpr uint8_t IS31FL3218_REG_UPDATE = 0x16;    // write any value: latch PWM + LED control
inline constexpr uint8_t IS31FL3218_REG_RESET = 0x17;     // write any value: reset all registers

inline constexpr uint8_t IS31FL3218_NORMAL_OPERATION = 0x01;
inline constexpr uint8_t IS31FL3218_LED_CTRL_ALL_ON = 0x3F;

class IS31FL3218;

class IS31FL3218Channel final : public output::FloatOutput, public Parented<IS31FL3218> {
 public:
  void set_channel(uint8_t channel) { this->channel_ = channel; }
  uint8_t get_channel() const { return this->channel_; }

 protected:
  void write_state(float state) override;

  uint8_t channel_{0};
};

class IS31FL3218 final : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void set_enable_pin(GPIOPin *pin) { this->enable_pin_ = pin; }
  void register_channel(IS31FL3218Channel *channel);
  /// Store a new PWM value; the I2C write happens in the next loop() (batched with the update latch)
  void set_channel_value(uint8_t channel, uint8_t value);

 protected:
  GPIOPin *enable_pin_{nullptr};
  uint8_t pwm_[IS31FL3218_NUM_CHANNELS]{};
  uint32_t dirty_mask_{0};  // bit n = channel n changed since the last update
};

}  // namespace esphome::is31fl3218
