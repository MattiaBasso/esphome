#pragma once

#include "esphome/components/audio_dac/audio_dac.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace max9860 {

enum MAX9860MicPreampGain {
  MAX9860_MIC_PREAMP_OFF = 0x00,
  MAX9860_MIC_PREAMP_0DB = 0x01,
  MAX9860_MIC_PREAMP_20DB = 0x02,
  MAX9860_MIC_PREAMP_30DB = 0x03,
};

enum MAX9860MicPGAGain {
  MAX9860_MIC_PGA_20DB = 0x00,
  MAX9860_MIC_PGA_19DB = 0x01,
  MAX9860_MIC_PGA_18DB = 0x02,
  MAX9860_MIC_PGA_17DB = 0x03,
  MAX9860_MIC_PGA_16DB = 0x04,
  MAX9860_MIC_PGA_15DB = 0x05,
  MAX9860_MIC_PGA_14DB = 0x06,
  MAX9860_MIC_PGA_13DB = 0x07,
  MAX9860_MIC_PGA_12DB = 0x08,
  MAX9860_MIC_PGA_11DB = 0x09,
  MAX9860_MIC_PGA_10DB = 0x0A,
  MAX9860_MIC_PGA_9DB = 0x0B,
  MAX9860_MIC_PGA_8DB = 0x0C,
  MAX9860_MIC_PGA_7DB = 0x0D,
  MAX9860_MIC_PGA_6DB = 0x0E,
  MAX9860_MIC_PGA_5DB = 0x0F,
  MAX9860_MIC_PGA_4DB = 0x10,
  MAX9860_MIC_PGA_3DB = 0x11,
  MAX9860_MIC_PGA_2DB = 0x12,
  MAX9860_MIC_PGA_1DB = 0x13,
  MAX9860_MIC_PGA_0DB = 0x14,
};

enum MAX9860SidetoneGain {
  MAX9860_SIDETONE_DISABLED = 0x00,
  MAX9860_SIDETONE_0DB = 0x01,
  MAX9860_SIDETONE_MINUS_2DB = 0x02,
  MAX9860_SIDETONE_MINUS_4DB = 0x03,
  MAX9860_SIDETONE_MINUS_6DB = 0x04,
  MAX9860_SIDETONE_MINUS_8DB = 0x05,
  MAX9860_SIDETONE_MINUS_10DB = 0x06,
  MAX9860_SIDETONE_MINUS_12DB = 0x07,
  MAX9860_SIDETONE_MINUS_14DB = 0x08,
  MAX9860_SIDETONE_MINUS_16DB = 0x09,
  MAX9860_SIDETONE_MINUS_18DB = 0x0A,
  MAX9860_SIDETONE_MINUS_20DB = 0x0B,
  MAX9860_SIDETONE_MINUS_22DB = 0x0C,
  MAX9860_SIDETONE_MINUS_24DB = 0x0D,
  MAX9860_SIDETONE_MINUS_26DB = 0x0E,
  MAX9860_SIDETONE_MINUS_28DB = 0x0F,
  MAX9860_SIDETONE_MINUS_30DB = 0x10,
  MAX9860_SIDETONE_MINUS_32DB = 0x11,
  MAX9860_SIDETONE_MINUS_34DB = 0x12,
  MAX9860_SIDETONE_MINUS_36DB = 0x13,
  MAX9860_SIDETONE_MINUS_38DB = 0x14,
  MAX9860_SIDETONE_MINUS_40DB = 0x15,
  MAX9860_SIDETONE_MINUS_42DB = 0x16,
  MAX9860_SIDETONE_MINUS_44DB = 0x17,
  MAX9860_SIDETONE_MINUS_46DB = 0x18,
  MAX9860_SIDETONE_MINUS_48DB = 0x19,
  MAX9860_SIDETONE_MINUS_50DB = 0x1A,
  MAX9860_SIDETONE_MINUS_52DB = 0x1B,
  MAX9860_SIDETONE_MINUS_54DB = 0x1C,
  MAX9860_SIDETONE_MINUS_56DB = 0x1D,
  MAX9860_SIDETONE_MINUS_58DB = 0x1E,
  MAX9860_SIDETONE_MINUS_60DB = 0x1F,
};

struct MAX9860ClockConfig {
  uint32_t mclk_freq;      // MCLK frequency in Hz
  uint32_t sample_rate;    // Sample rate in Hz
  uint8_t psclk;          // MCLK prescaler (PSCLK bits)
  uint8_t freq_mode;      // Integer clock mode (FREQ bits)
  uint8_t use_16khz;      // 16KHZ bit value
  uint16_t nhi_nlo;       // Combined NHI and NLO for normal mode
};

class MAX9860 : public audio_dac::AudioDac, public Component, public i2c::I2CDevice {
 public:
  /////////////////////////
  // Component overrides //
  /////////////////////////

  void setup() override;
  void dump_config() override;

  ////////////////////////
  // AudioDac overrides //
  ////////////////////////

  /// @brief Writes the volume out to the DAC
  /// @param volume floating point between 0.0 and 1.0
  /// @return True if successful and false otherwise
  bool set_volume(float volume) override;

  /// @brief Gets the current volume out from the DAC
  /// @return floating point between 0.0 and 1.0
  float volume() override;

  /// @brief Disables mute for audio out
  /// @return True if successful and false otherwise
  bool set_mute_off() override { return this->set_mute_state_(false); }

  /// @brief Enables mute for audio out
  /// @return True if successful and false otherwise
  bool set_mute_on() override { return this->set_mute_state_(true); }

  bool is_muted() override { return this->is_muted_; }

  //////////////////////////////////
  // MAX9860 configuration setters //
  //////////////////////////////////

  void set_sample_frequency(uint32_t sample_frequency) { this->sample_frequency_ = sample_frequency; }
  void set_mic_pga_gain(MAX9860MicPGAGain mic_pga_gain) { this->mic_pga_gain_ = mic_pga_gain; }
  void set_use_microphone(bool use_microphone) { this->use_microphone_ = use_microphone; }
  void set_master_mode(bool master_mode) { this->master_mode_ = master_mode; }
  void set_pll_mode(bool pll_mode) { this->pll_mode_ = pll_mode; }
  void set_left_adc_enable(bool left_adc_enable) { this->left_adc_enable_ = left_adc_enable; }
  void set_right_adc_enable(bool right_adc_enable) { this->right_adc_enable_ = right_adc_enable; }
  void set_agc_enable(bool agc_enable) { this->agc_enable_ = agc_enable; }
  void set_sidetone_gain(MAX9860SidetoneGain sidetone_gain) { this->sidetone_gain_ = sidetone_gain; }

 protected:
  /// @brief Gets the appropriate clock configuration for the given MCLK and sample rate
  /// @param mclk_freq MCLK frequency in Hz
  /// @param sample_rate Sample rate in Hz
  /// @return Pointer to MAX9860ClockConfig or nullptr if not supported
  static const MAX9860ClockConfig *get_clock_config(uint32_t mclk_freq, uint32_t sample_rate);

  /// @brief Configures the MAX9860 clocking registers
  /// @return True if successful and false otherwise
  bool configure_clock_();

  /// @brief Configures the MAX9860 digital audio interface
  /// @return True if successful and false otherwise
  bool configure_digital_interface_();

  /// @brief Configures the MAX9860 microphone settings
  /// @return True if successful and false otherwise
  bool configure_microphone_();

  /// @brief Configures power management settings
  /// @return True if successful and false otherwise
  bool configure_power_management_();

  /// @brief Configures digital level controls and filters
  /// @return True if successful and false otherwise
  bool configure_digital_controls_();

  /// @brief Mutes or unmutes the DAC audio out
  /// @param mute_state True to mute, false to unmute
  /// @return True if successful and false otherwise
  bool set_mute_state_(bool mute_state);

  // Configuration parameters
  uint32_t sample_frequency_{8000};
  MAX9860MicPGAGain mic_pga_gain_{MAX9860_MIC_PGA_0DB};
  MAX9860SidetoneGain sidetone_gain_{MAX9860_SIDETONE_DISABLED};
  
  bool use_microphone_{false};
  bool master_mode_{true};
  bool pll_mode_{false};
  bool left_adc_enable_{true};
  bool right_adc_enable_{false};
  bool agc_enable_{false};
  bool is_muted_{false};

  // Default MCLK frequency (can be overridden)
  uint32_t mclk_frequency_{12000000};  // 12MHz default
};

}  // namespace max9860
}  // namespace esphome