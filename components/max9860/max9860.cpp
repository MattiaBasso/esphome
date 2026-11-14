#include "max9860.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <cinttypes>
#include <cmath>

namespace esphome::max9860 {

static const char *const TAG = "max9860";

// Mark the component as failed; use only in setup
#define MAX9860_ERROR_FAILED(func) \
  if (!(func)) { \
    this->mark_failed(); \
    return; \
  }

void MAX9860::setup() {
  uint8_t revision = 0;
  if (!this->read_byte(MAX9860_REG_REVISION, &revision) || revision != MAX9860_REVISION) {
    ESP_LOGE(TAG, "Unexpected revision code 0x%02X (expected 0x%02X)", revision, MAX9860_REVISION);
    this->mark_failed();
    return;
  }

  // MCLK prescaler: the internal PCLK must be 10-20 MHz (datasheet Table 3)
  uint32_t divider;
  if (this->mclk_frequency_ <= 20000000) {
    this->psclk_ = 0x01;
    divider = 1;
  } else if (this->mclk_frequency_ <= 40000000) {
    this->psclk_ = 0x02;
    divider = 2;
  } else {
    this->psclk_ = 0x03;
    divider = 4;
  }
  const double pclk = static_cast<double>(this->mclk_frequency_) / divider;

  // Normal mode LRCLK divider: N = 65536 * 96 * fLRCLK / fPCLK (15 bits)
  const double n = std::round(65536.0 * 96.0 * this->sample_rate_ / pclk);
  if (n < 1 || n > 0x7FFF) {
    ESP_LOGE(TAG, "N divider out of range for MCLK %" PRIu32 " Hz / %" PRIu32 " Hz", this->mclk_frequency_,
             this->sample_rate_);
    this->mark_failed();
    return;
  }
  this->n_divider_ = static_cast<uint16_t>(n);

  // 16KHZ bit selects the AGC clock rate when FREQ = 00
  const uint8_t sys_clock = (this->psclk_ << 4) | (this->sample_rate_ > 24000 ? 0x01 : 0x00);

  // Voice high-pass filter on the ADC, only defined for some sample rates (Table 7)
  uint8_t avflt = 0x0;
  if (this->sample_rate_ == 16000) {
    avflt = 0x1;  // elliptical high-pass + 217 Hz notch
  } else if (this->sample_rate_ == 8000) {
    avflt = 0x3;
  } else if (this->sample_rate_ == 48000) {
    avflt = 0x5;
  }

  uint8_t power = MAX9860_PWR_SHDN | MAX9860_PWR_DACEN;
  if (this->use_microphone_)
    power |= MAX9860_PWR_ADCLEN;

  // Configure while in shutdown (registers are retained in shutdown), then power up with the DAC muted
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_PWR_MAN, 0x00));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_INT_ENABLE, 0x00));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_SYS_CLOCK, sys_clock));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_CLOCK_HIGH, (this->n_divider_ >> 8) & 0x7F));  // PLL = 0
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_CLOCK_LOW, this->n_divider_ & 0xFF));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_IFACE_1, MAX9860_IFACE_DDLY));  // slave, I2S
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_IFACE_2, MAX9860_IFACE_ADLY));  // I2S, mono
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_VOICE_FILTER, avflt << 4));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_DAC_ATTEN, MAX9860_DVA_MUTE));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_ADC_LEVEL, MAX9860_ADC_LEVEL_0DB));
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_DAC_GAIN, 0x00));  // 0 dB, sidetone off
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_MIC_GAIN, this->mic_gain_reg_()));
  if (this->agc_enabled_) {
    MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_MIC_AGC, MAX9860_AGC_DEFAULT));
    // AGCTH: 0x0 = -3 dBFS ... 0xF = -18 dBFS; noise gate off
    MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_NOISE_GATE, (-this->agc_target_dbfs_ - 3) & 0x0F));
  } else {
    MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_MIC_AGC, 0x00));
    MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_NOISE_GATE, 0x00));
  }
  MAX9860_ERROR_FAILED(this->write_checked_(MAX9860_REG_PWR_MAN, power));

  this->is_muted_ = true;
  this->configured_ = true;

  if (this->irq_pin_ == nullptr) {
    this->disable_loop();
    return;
  }
  // IRQ: clip detect + PLL unlock (not SLD, which fires at the end of every volume ramp). Clear stale flags
  // first; the ISR wakes loop() on each falling edge, loop() reads (and so clears) the status register.
  uint8_t status = 0;
  this->read_byte(MAX9860_REG_INT_STATUS, &status);
  this->irq_pin_->setup();
  this->irq_pin_->attach_interrupt(&MAX9860::gpio_intr, this, gpio::INTERRUPT_FALLING_EDGE);
  if (!this->write_byte(MAX9860_REG_INT_ENABLE, MAX9860_STATUS_CLD | MAX9860_STATUS_ULK)) {
    ESP_LOGW(TAG, "Could not enable the interrupts");
  }
  // Keep the loop running if IRQ is already low; otherwise the ISR re-enables it
  if (this->irq_pin_->digital_read())
    this->disable_loop();
}

void IRAM_ATTR MAX9860::gpio_intr(MAX9860 *arg) { arg->enable_loop_soon_any_context(); }

void MAX9860::loop() {
  const uint32_t now = millis();
  const bool irq_asserted = !this->irq_pin_->digital_read();
  if (!irq_asserted && !this->pll_unlocked_) {
    this->log_clip_summary_(now, true);
    this->disable_loop();
    return;
  }
  // Rate limit: a clip burst re-asserts IRQ continuously; while the PLL is unlocked re-check every 500 ms
  if (now - this->last_status_read_ms_ < (this->pll_unlocked_ ? 500u : 100u))
    return;
  this->last_status_read_ms_ = now;
  uint8_t status = 0;
  if (!this->read_byte(MAX9860_REG_INT_STATUS, &status)) {
    this->status_set_warning();
    return;
  }
  this->status_clear_warning();
  this->handle_status_(status, now);
}

void MAX9860::handle_status_(uint8_t status, uint32_t now) {
  if (status & MAX9860_STATUS_ULK) {
    if (!this->pll_unlocked_) {
      this->pll_unlocked_ = true;
      this->unlock_start_ms_ = now;
      if (this->pll_locked_once_) {
        ESP_LOGW(TAG, "PLL unlocked: audio data not reliable");
        this->pll_unlock_callback_.call();
      } else {
        ESP_LOGD(TAG, "PLL not locked yet (waiting for the I2S clocks)");
      }
      this->enable_loop();  // re-check until it locks
    }
  } else if (this->pll_unlocked_) {
    this->pll_unlocked_ = false;
    if (this->pll_locked_once_) {
      ESP_LOGI(TAG, "PLL locked again after %" PRIu32 " ms", now - this->unlock_start_ms_);
    } else {
      ESP_LOGI(TAG, "PLL locked");
    }
    this->pll_locked_once_ = true;
  }

  if (status & MAX9860_STATUS_CLD) {
    this->clip_count_++;
    if (!this->clip_triggered_once_ || now - this->last_clip_trigger_ms_ >= 1000) {
      this->clip_triggered_once_ = true;
      this->last_clip_trigger_ms_ = now;
      this->clip_callback_.call();
    }
    this->log_clip_summary_(now, false);
  }
}

void MAX9860::log_clip_summary_(uint32_t now, bool force) {
  if (this->clip_count_ == 0)
    return;
  // The first event of a burst is logged at once, then at most one line every 5 s
  if (!force && this->last_clip_summary_ms_ != 0 && now - this->last_clip_summary_ms_ < 5000)
    return;
  ESP_LOGI(TAG, "Clip detected (ADC/DAC, or mic AGC at 0 dB): %u event(s)", this->clip_count_);
  this->clip_count_ = 0;
  this->last_clip_summary_ms_ = now;
}

uint8_t MAX9860::mic_gain_reg_() const {
  if (!this->use_microphone_)
    return 0x00;  // PAM = 00 disables mic inputs and bias
  return (this->mic_preamp_gain_ << 5) | (MAX9860_PGAM_0DB - this->mic_pga_gain_db_);
}

void MAX9860::apply_mic_settings_() {
  if (!this->configured_ || !this->use_microphone_)
    return;  // setup() writes the stored values
  bool ok = this->write_byte(MAX9860_REG_MIC_GAIN, this->mic_gain_reg_());
  if (this->agc_enabled_) {
    ok &= this->write_byte(MAX9860_REG_NOISE_GATE, (-this->agc_target_dbfs_ - 3) & 0x0F);
  }
  static const uint8_t PREAMP_DB[] = {0, 0, 20, 30};
  ESP_LOGD(TAG, "Mic preamp %u dB, PGA %u dB, AGC target %d dBFS%s", PREAMP_DB[this->mic_preamp_gain_ & 0x03],
           this->mic_pga_gain_db_, this->agc_target_dbfs_, ok ? "" : " (I2C write failed)");
}

void MAX9860::dump_config() {
  ESP_LOGCONFIG(TAG,
                "MAX9860 Audio Codec:\n"
                "  MCLK: %" PRIu32 " Hz (PSCLK %u)\n"
                "  Sample Rate: %" PRIu32 " Hz (N = 0x%04X)\n"
                "  Use Microphone: %s",
                this->mclk_frequency_, this->psclk_, this->sample_rate_, this->n_divider_,
                YESNO(this->use_microphone_));
  if (this->use_microphone_) {
    static const uint8_t PREAMP_DB[] = {0, 0, 20, 30};
    ESP_LOGCONFIG(TAG, "  Mic Preamp: %u dB, PGA: %u dB", PREAMP_DB[this->mic_preamp_gain_ & 0x03],
                  this->mic_pga_gain_db_);
    if (this->agc_enabled_) {
      ESP_LOGCONFIG(TAG, "  AGC target: %d dBFS", this->agc_target_dbfs_);
    }
  }
  ESP_LOGCONFIG(TAG, "  Volume range: %d..0 dB", this->min_volume_db_);
  LOG_PIN("  IRQ Pin: ", this->irq_pin_);
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Failed to initialize!");
  }
}

void MAX9860::on_audio_started() {
  // The ULK flag is latched while clocks are absent; reading the status register clears it
  uint8_t status = 0;
  if (this->read_byte(MAX9860_REG_INT_STATUS, &status)) {
    ESP_LOGD(TAG, "Audio clocks started (status 0x%02X)", status);
    if (this->irq_pin_ != nullptr)
      this->handle_status_(status, millis());  // the read cleared the flags: don't lose the events
  }
}

bool MAX9860::set_volume(float volume) {
  this->volume_ = clamp(volume, 0.0f, 1.0f);
  if (this->is_muted_)
    return true;  // applied on unmute
  return this->write_byte(MAX9860_REG_DAC_ATTEN, this->volume_to_dva_());
}

bool MAX9860::set_mute_state_(bool mute) {
  const uint8_t dva = mute ? MAX9860_DVA_MUTE : this->volume_to_dva_();
  if (!this->write_byte(MAX9860_REG_DAC_ATTEN, dva))
    return false;
  this->is_muted_ = mute;
  return true;
}

uint8_t MAX9860::volume_to_dva_() const {
  if (this->volume_ <= 0.0f)
    return MAX9860_DVA_MUTE;
  // Linear in dB between min_volume_db (volume -> 0) and 0 dB (volume = 1)
  const float attenuation_db = (1.0f - this->volume_) * static_cast<float>(-this->min_volume_db_);
  const int dva = MAX9860_DVA_0DB + 2 * static_cast<int>(std::lround(attenuation_db));
  return static_cast<uint8_t>(std::min(dva, MAX9860_DVA_MUTE - 2));
}

bool MAX9860::write_checked_(uint8_t reg, uint8_t value) {
  uint8_t readback = 0;
  if (!this->write_byte(reg, value) || !this->read_byte(reg, &readback) || readback != value) {
    ESP_LOGE(TAG, "Register 0x%02X: wrote 0x%02X, read back 0x%02X", reg, value, readback);
    return false;
  }
  return true;
}

}  // namespace esphome::max9860
