#include "max9860.h"
#include "max9860_const.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <cinttypes>

namespace esphome {
namespace max9860 {

static const char *const TAG = "max9860";

// Mark the component as failed; use only in setup
#define MAX9860_ERROR_FAILED(func) \
  if (!(func)) { \
    this->mark_failed(); \
    return; \
  }
// Return false; use outside of setup
#define MAX9860_ERROR_CHECK(func) \
  if (!(func)) { \
    return false; \
  }

void MAX9860::setup() {
  // Enable the device by bringing it out of shutdown
  MAX9860_ERROR_FAILED(this->write_byte(MAX9860_REG_SYSTEM_SHUTDOWN, 0x80));  // SHDN=1

  // Small delay to allow device to initialize
  delay(10);

  // Configure the device
  MAX9860_ERROR_FAILED(this->configure_clock_());
  MAX9860_ERROR_FAILED(this->configure_digital_interface_());
  MAX9860_ERROR_FAILED(this->configure_microphone_());
  MAX9860_ERROR_FAILED(this->configure_digital_controls_());
  MAX9860_ERROR_FAILED(this->configure_power_management_());

  // Set initial volume to 75%
  this->set_volume(0.75);

  // Additional delay to ensure all settings are applied
  delay(25);
}

void MAX9860::dump_config() {
  ESP_LOGCONFIG(TAG,
                "MAX9860 Audio Codec:\n"
                "  Sample Rate: %" PRIu32 " Hz\n"
                "  Master Mode: %s\n"
                "  PLL Mode: %s\n"
                "  Use Microphone: %s\n"
                "  Left ADC: %s\n"
                "  Right ADC: %s\n"
                "  AGC Enable: %s",
                this->sample_frequency_, YESNO(this->master_mode_), YESNO(this->pll_mode_), 
                YESNO(this->use_microphone_), YESNO(this->left_adc_enable_), 
                YESNO(this->right_adc_enable_), YESNO(this->agc_enable_));

  if (this->is_failed()) {
    ESP_LOGCONFIG(TAG, "  Failed to initialize!");
    return;
  }
}

bool MAX9860::set_volume(float volume) {
  volume = clamp(volume, 0.0f, 1.0f);
  
  // MAX9860 uses DVA register for digital volume control
  // Range: +3dB to mute, with 0.5dB steps (register values 0x00-0xBC)
  // Convert 0.0-1.0 to register value
  uint8_t reg_value;
  if (volume == 0.0f) {
    reg_value = 0xBC;  // Mute
  } else {
    // Map 0.0-1.0 to 0x06 (0dB) to 0x00 (+3dB)
    reg_value = static_cast<uint8_t>(6 - (volume * 6));
    reg_value = reg_value << 1;  // LSB is always 0
  }
  
  return this->write_byte(MAX9860_REG_DAC_ATTENUATION, reg_value);
}

float MAX9860::volume() {
  uint8_t reg_value;
  if (!this->read_byte(MAX9860_REG_DAC_ATTENUATION, &reg_value))
    return 0.0f;
  
  if (reg_value >= 0xBC) {
    return 0.0f;  // Muted
  }
  
  // Convert register value back to 0.0-1.0 range
  reg_value = reg_value >> 1;  // Remove LSB
  return (6.0f - reg_value) / 6.0f;
}

const MAX9860ClockConfig *MAX9860::get_clock_config(uint32_t mclk_freq, uint32_t sample_rate) {
  for (const auto &config : MAX9860_CLOCK_CONFIGS) {
    if (config.mclk_freq == mclk_freq && config.sample_rate == sample_rate) {
      return &config;
    }
  }
  return nullptr;
}

bool MAX9860::configure_clock_() {
  const MAX9860ClockConfig *config = get_clock_config(this->mclk_frequency_, this->sample_frequency_);
  
  if (config == nullptr) {
    ESP_LOGE(TAG, "Unsupported MCLK/sample rate combination: %" PRIu32 "Hz/%" PRIu32 "Hz", 
             this->mclk_frequency_, this->sample_frequency_);
    return false;
  }

  // Configure system clock register
  uint8_t sys_clock = 0x00;
  sys_clock |= (config->psclk & 0x03) << 4;      // PSCLK bits [5:4]
  sys_clock |= (config->freq_mode & 0x03) << 1;  // FREQ bits [2:1]
  sys_clock |= (config->use_16khz & 0x01);       // 16KHZ bit [0]
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_SYSTEM_CLOCK, sys_clock));

  // If using normal mode (FREQ = 00), configure NHI and NLO registers
  if (config->freq_mode == 0x00) {
    uint8_t pll_enable = this->pll_mode_ ? 0x80 : 0x00;
    uint8_t nhi = pll_enable | ((config->nhi_nlo >> 8) & 0x7F);
    uint8_t nlo = config->nhi_nlo & 0xFF;
    
    MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_AUDIO_CLOCK_HIGH, nhi));
    MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_AUDIO_CLOCK_LOW, nlo));
  }

  return true;
}

bool MAX9860::configure_digital_interface_() {
  // Configure Interface register 1 (0x06)
  uint8_t interface1 = 0x00;
  
  if (this->master_mode_) {
    interface1 |= 0x80;  // MAS = 1 for master mode
  }
  
  // Set BCLK select for master mode (48x LRCLK is typical)
  interface1 |= 0x02;  // BSEL = 010 (48x LRCLK)
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_INTERFACE1, interface1));

  // Configure Interface register 2 (0x07)  
  uint8_t interface2 = 0x00;
  
  // Enable stereo mode if right ADC is enabled
  if (this->right_adc_enable_) {
    interface2 |= 0x08;  // ST = 1 for stereo
  }
  
  // Set BSEL for master mode operation
  interface2 |= 0x02;  // BSEL = 010
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_INTERFACE2, interface2));

  return true;
}

bool MAX9860::configure_microphone_() {
  if (!this->use_microphone_) {
    // Disable microphone preamp if not using microphone
    MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_MIC_GAIN, 0x00));
    return true;
  }

  // Configure microphone gain register (0x0C)
  uint8_t mic_gain = 0x00;
  
  // Set preamp gain (default to +20dB for good sensitivity)
  mic_gain |= (MAX9860_MIC_PREAMP_20DB & 0x03) << 5;  // PAM bits [6:5]
  
  // Set PGA gain
  mic_gain |= (this->mic_pga_gain_ & 0x1F);  // PGAM bits [4:0]
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_MIC_GAIN, mic_gain));

  // Configure AGC if enabled
  if (this->agc_enable_) {
    // Configure AGC register (0x0E)
    uint8_t agc_config = 0x00;
    agc_config |= 0x00 << 7;    // AGCSRC = 0 (use left ADC)
    agc_config |= 0x03 << 4;    // AGCRLS = 011 (625ms release time)
    agc_config |= 0x02 << 2;    // AGCATK = 10 (50ms attack time)
    agc_config |= 0x02;         // AGCHLD = 10 (100ms hold time)
    
    MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_MIC_AGC, agc_config));
    
    // Configure AGC threshold (0x0F)
    uint8_t agc_threshold = 0x00;
    agc_threshold |= 0x08 << 4;  // ANTH = 1000 (-44dB noise gate threshold)
    agc_threshold |= 0x06;       // AGCTH = 0110 (-9dB AGC threshold)
    
    MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_NOISE_GATE_AGC_THRESHOLD, agc_threshold));
  }

  return true;
}

bool MAX9860::configure_digital_controls_() {
  // Configure digital filters based on sample rate
  uint8_t filter_config = 0x00;
  
  if (this->sample_frequency_ == 8000) {
    filter_config = (MAX9860_FILTER_ELLIPTICAL_8KHZ << 4) | MAX9860_FILTER_ELLIPTICAL_8KHZ;
  } else if (this->sample_frequency_ == 16000) {
    filter_config = (MAX9860_FILTER_ELLIPTICAL_16KHZ << 4) | MAX9860_FILTER_ELLIPTICAL_16KHZ;
  } else if (this->sample_frequency_ >= 44100) {
    filter_config = (MAX9860_FILTER_BUTTERWORTH_48KHZ << 4) | MAX9860_FILTER_BUTTERWORTH_48KHZ;
  } else {
    filter_config = 0x00;  // Disable filters for other rates
  }
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_VOICE_FILTER, filter_config));

  // Configure ADC output levels (0x0A)
  uint8_t adc_levels = 0x33;  // 0dB gain for both channels
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_ADC_OUTPUT_LEVELS, adc_levels));

  // Configure DAC gain and sidetone (0x0B)
  uint8_t dac_gain_sidetone = 0x00;
  dac_gain_sidetone |= 0x00 << 4;                    // DVG = 00 (0dB additional gain)
  dac_gain_sidetone |= (this->sidetone_gain_ & 0x1F); // DVST sidetone gain
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_DAC_GAIN_SIDETONE, dac_gain_sidetone));

  return true;
}

bool MAX9860::configure_power_management_() {
  // Configure power management register (0x10)
  uint8_t power_config = 0x80;  // SHDN = 1 (device enabled)
  
  // Enable DAC
  power_config |= 0x08;  // DACEN = 1
  
  // Enable ADCs based on configuration
  if (this->left_adc_enable_) {
    power_config |= 0x02;  // ADCLEN = 1
  }
  
  if (this->right_adc_enable_) {
    power_config |= 0x01;  // ADCREN = 1
  }
  
  MAX9860_ERROR_CHECK(this->write_byte(MAX9860_REG_SYSTEM_SHUTDOWN, power_config));

  return true;
}

bool MAX9860::set_mute_state_(bool mute_state) {
  this->is_muted_ = mute_state;
  
  if (mute_state) {
    // Set volume to mute
    return this->write_byte(MAX9860_REG_DAC_ATTENUATION, 0xBC);
  } else {
    // Restore previous volume (assume 0dB for now)
    return this->write_byte(MAX9860_REG_DAC_ATTENUATION, 0x06);
  }
}