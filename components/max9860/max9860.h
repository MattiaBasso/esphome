#pragma once

#include "esphome/components/audio_dac/audio_dac.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

#include <algorithm>

namespace esphome::max9860 {

// Register map (datasheet Table 1)
inline constexpr uint8_t MAX9860_REG_INT_STATUS = 0x00;    // CLD SLD ULK (read clears)
inline constexpr uint8_t MAX9860_REG_INT_ENABLE = 0x02;
inline constexpr uint8_t MAX9860_REG_SYS_CLOCK = 0x03;     // PSCLK[5:4] FREQ[2:1] 16KHZ[0]
inline constexpr uint8_t MAX9860_REG_CLOCK_HIGH = 0x04;    // PLL[7] NHI[6:0]
inline constexpr uint8_t MAX9860_REG_CLOCK_LOW = 0x05;     // NLO[7:0]
inline constexpr uint8_t MAX9860_REG_IFACE_1 = 0x06;       // MAS WCI DBCI DDLY HIZ TDM
inline constexpr uint8_t MAX9860_REG_IFACE_2 = 0x07;       // ABCI ADLY ST BSEL[2:0]
inline constexpr uint8_t MAX9860_REG_VOICE_FILTER = 0x08;  // AVFLT[7:4] DVFLT[3:0]
inline constexpr uint8_t MAX9860_REG_DAC_ATTEN = 0x09;     // DVA
inline constexpr uint8_t MAX9860_REG_ADC_LEVEL = 0x0A;     // ADCRL[7:4] ADCLL[3:0]
inline constexpr uint8_t MAX9860_REG_DAC_GAIN = 0x0B;      // DVG[6:5] DVST[4:0]
inline constexpr uint8_t MAX9860_REG_MIC_GAIN = 0x0C;      // PAM[6:5] PGAM[4:0]
inline constexpr uint8_t MAX9860_REG_MIC_AGC = 0x0E;       // AGCSRC AGCRLS AGCATK AGCHLD
inline constexpr uint8_t MAX9860_REG_NOISE_GATE = 0x0F;    // ANTH[7:4] AGCTH[3:0]
inline constexpr uint8_t MAX9860_REG_PWR_MAN = 0x10;       // SHDN DACEN ADCLEN ADCREN
inline constexpr uint8_t MAX9860_REG_REVISION = 0xFF;

inline constexpr uint8_t MAX9860_REVISION = 0x40;

// Registers 0x00 (status, read clears) and 0x02 (interrupt enable) share the bit layout
inline constexpr uint8_t MAX9860_STATUS_CLD = 0x80;  // clip detect (ADC/DAC), or AGC at 0 dB with no headroom
inline constexpr uint8_t MAX9860_STATUS_SLD = 0x40;  // slew level detect (volume/gain ramp done)
inline constexpr uint8_t MAX9860_STATUS_ULK = 0x20;  // digital PLL unlocked: audio data not reliable
inline constexpr uint8_t MAX9860_IFACE_DDLY = 0x10;  // I2S: SDIN latched on the 2nd BCLK edge
inline constexpr uint8_t MAX9860_IFACE_ADLY = 0x10;  // I2S: SDOUT valid on the 2nd BCLK edge
inline constexpr uint8_t MAX9860_PWR_SHDN = 0x80;
inline constexpr uint8_t MAX9860_PWR_DACEN = 0x08;
inline constexpr uint8_t MAX9860_PWR_ADCLEN = 0x02;

inline constexpr uint8_t MAX9860_DVA_0DB = 0x06;  // +2 per dB of attenuation
inline constexpr uint8_t MAX9860_DVA_MUTE = 0xBC;
inline constexpr uint8_t MAX9860_ADC_LEVEL_0DB = 0x33;
inline constexpr uint8_t MAX9860_PGAM_0DB = 0x14;  // PGAM = 0x14 - gain_dB
// AGC: release 625 ms, attack 12 ms, hold 50 ms (AGCSRC = left ADC)
inline constexpr uint8_t MAX9860_AGC_DEFAULT = 0x35;

enum MAX9860PreampGain : uint8_t {
  MAX9860_PREAMP_GAIN_0DB = 0x01,
  MAX9860_PREAMP_GAIN_20DB = 0x02,
  MAX9860_PREAMP_GAIN_30DB = 0x03,
};

/// MAX9860 as I2S slave, Normal clock mode (N divider, PLL off). The I2S master must provide an MCLK
/// synchronous with LRCLK (e.g. ESP32 mclk_multiple 768 at 16 kHz -> 12.288 MHz).
class MAX9860 final : public audio_dac::AudioDac, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // AudioDac
  bool set_volume(float volume) override;
  float volume() override { return this->volume_; }
  bool set_mute_off() override { return this->set_mute_state_(false); }
  bool set_mute_on() override { return this->set_mute_state_(true); }
  bool is_muted() override { return this->is_muted_; }
  void on_audio_started() override;

  // Configuration
  void set_mclk_frequency(uint32_t mclk_frequency) { this->mclk_frequency_ = mclk_frequency; }
  void set_sample_rate(uint32_t sample_rate) { this->sample_rate_ = sample_rate; }
  void set_use_microphone(bool use_microphone) { this->use_microphone_ = use_microphone; }
  // Microphone settings: also usable at runtime (applied immediately once the codec is set up)
  void set_mic_preamp_gain(MAX9860PreampGain gain) {
    this->mic_preamp_gain_ = gain;
    this->apply_mic_settings_();
  }
  /// Runtime helper: 0, 20 or 30 dB (other values round down)
  void set_mic_preamp_gain_db(uint8_t gain_db) {
    this->set_mic_preamp_gain(gain_db >= 30   ? MAX9860_PREAMP_GAIN_30DB
                              : gain_db >= 20 ? MAX9860_PREAMP_GAIN_20DB
                                              : MAX9860_PREAMP_GAIN_0DB);
  }
  void set_mic_pga_gain(uint8_t gain_db) {
    this->mic_pga_gain_db_ = std::min<uint8_t>(gain_db, 20);
    this->apply_mic_settings_();
  }
  /// Enables the AGC; target -18..-3 dBFS
  void set_agc_target(int8_t target_dbfs) {
    this->agc_enabled_ = true;
    this->agc_target_dbfs_ = std::max<int8_t>(-18, std::min<int8_t>(-3, target_dbfs));
    this->apply_mic_settings_();
  }
  void set_min_volume_db(int8_t min_volume_db) { this->min_volume_db_ = min_volume_db; }

  /// Optional IRQ pin (open drain, active low). Enables the clip detect and PLL unlock interrupts.
  void set_irq_pin(InternalGPIOPin *pin) { this->irq_pin_ = pin; }
  void add_on_clip_callback(std::function<void()> &&callback) { this->clip_callback_.add(std::move(callback)); }
  void add_on_pll_unlock_callback(std::function<void()> &&callback) {
    this->pll_unlock_callback_.add(std::move(callback));
  }

 protected:
  static void IRAM_ATTR gpio_intr(MAX9860 *arg);
  void handle_status_(uint8_t status, uint32_t now);
  void log_clip_summary_(uint32_t now, bool force);

  bool write_checked_(uint8_t reg, uint8_t value);
  uint8_t mic_gain_reg_() const;
  void apply_mic_settings_();
  bool set_mute_state_(bool mute);
  uint8_t volume_to_dva_() const;

  uint32_t mclk_frequency_{12288000};
  uint32_t sample_rate_{16000};
  bool use_microphone_{true};
  MAX9860PreampGain mic_preamp_gain_{MAX9860_PREAMP_GAIN_20DB};
  uint8_t mic_pga_gain_db_{0};
  bool agc_enabled_{false};
  int8_t agc_target_dbfs_{-12};
  int8_t min_volume_db_{-50};

  // Derived in setup()
  uint8_t psclk_{0};
  uint16_t n_divider_{0};
  bool configured_{false};

  float volume_{0.7f};

  // IRQ
  InternalGPIOPin *irq_pin_{nullptr};
  uint32_t last_status_read_ms_{0};
  uint32_t last_clip_summary_ms_{0};
  uint32_t last_clip_trigger_ms_{0};
  uint32_t unlock_start_ms_{0};
  uint16_t clip_count_{0};          // clip events since the last log line
  bool clip_triggered_once_{false};
  bool pll_unlocked_{false};        // last status read had ULK: re-read until it clears
  bool pll_locked_once_{false};     // before the first lock (no I2S clocks yet) ULK is expected
  CallbackManager<void()> clip_callback_;
  CallbackManager<void()> pll_unlock_callback_;
};

class ClipTrigger : public Trigger<> {
 public:
  explicit ClipTrigger(MAX9860 *parent) {
    parent->add_on_clip_callback([this]() { this->trigger(); });
  }
};

class PllUnlockTrigger : public Trigger<> {
 public:
  explicit PllUnlockTrigger(MAX9860 *parent) {
    parent->add_on_pll_unlock_callback([this]() { this->trigger(); });
  }
};

}  // namespace esphome::max9860
