#pragma once

#include "max9860.h"

namespace esphome {
namespace max9860 {

// MAX9860 I2C Register Addresses
static const uint8_t MAX9860_REG_STATUS = 0x00;               // Interrupt Status
static const uint8_t MAX9860_REG_MIC_AGC_READBACK = 0x01;     // Microphone NG/AGC Readback
static const uint8_t MAX9860_REG_INTERRUPT_ENABLE = 0x02;     // Interrupt Enable
static const uint8_t MAX9860_REG_SYSTEM_CLOCK = 0x03;         // System Clock
static const uint8_t MAX9860_REG_AUDIO_CLOCK_HIGH = 0x04;     // Stereo Audio Clock Control High
static const uint8_t MAX9860_REG_AUDIO_CLOCK_LOW = 0x05;      // Stereo Audio Clock Control Low
static const uint8_t MAX9860_REG_INTERFACE1 = 0x06;           // Interface
static const uint8_t MAX9860_REG_INTERFACE2 = 0x07;           // Interface
static const uint8_t MAX9860_REG_VOICE_FILTER = 0x08;         // Voice Filter
static const uint8_t MAX9860_REG_DAC_ATTENUATION = 0x09;      // DAC Attenuation
static const uint8_t MAX9860_REG_ADC_OUTPUT_LEVELS = 0x0A;    // ADC Output Levels
static const uint8_t MAX9860_REG_DAC_GAIN_SIDETONE = 0x0B;    // DAC Gain and Sidetone
static const uint8_t MAX9860_REG_MIC_GAIN = 0x0C;             // Microphone Gain
static const uint8_t MAX9860_REG_RESERVED = 0x0D;             // Reserved
static const uint8_t MAX9860_REG_MIC_AGC = 0x0E;              // Microphone AGC
static const uint8_t MAX9860_REG_NOISE_GATE_AGC_THRESHOLD = 0x0F; // Noise Gate, Microphone AGC
static const uint8_t MAX9860_REG_SYSTEM_SHUTDOWN = 0x10;      // System Shutdown
static const uint8_t MAX9860_REG_REVISION = 0xFF;             // Revision Code

// Clock Configuration Table for common MCLK and sample rate combinations
static const MAX9860ClockConfig MAX9860_CLOCK_CONFIGS[] = {
    // MCLK = 12MHz configurations (exact integer mode)
    {12000000, 8000, 0x01, 0x01, 0, 0},     // 12MHz, 8kHz, PSCLK=01, FREQ=01, 16KHZ=0
    {12000000, 16000, 0x01, 0x01, 1, 0},    // 12MHz, 16kHz, PSCLK=01, FREQ=01, 16KHZ=1
    {12000000, 24000, 0x01, 0x00, 0, 2000}, // 12MHz, 24kHz, normal mode
    {12000000, 32000, 0x01, 0x00, 0, 1500}, // 12MHz, 32kHz, normal mode
    {12000000, 44100, 0x01, 0x00, 0, 1088}, // 12MHz, 44.1kHz, normal mode
    {12000000, 48000, 0x01, 0x00, 0, 1000}, // 12MHz, 48kHz, normal mode
    
    // MCLK = 13MHz configurations (exact integer mode)
    {13000000, 8000, 0x01, 0x02, 0, 0},     // 13MHz, 8kHz, PSCLK=01, FREQ=10, 16KHZ=0
    {13000000, 16000, 0x01, 0x02, 1, 0},    // 13MHz, 16kHz, PSCLK=01, FREQ=10, 16KHZ=1
    {13000000, 24000, 0x01, 0x00, 0, 2167}, // 13MHz, 24kHz, normal mode
    {13000000, 32000, 0x01, 0x00, 0, 1625}, // 13MHz, 32kHz, normal mode
    {13000000, 44100, 0x01, 0x00, 0, 1179}, // 13MHz, 44.1kHz, normal mode
    {13000000, 48000, 0x01, 0x00, 0, 1083}, // 13MHz, 48kHz, normal mode
    
    // MCLK = 19.2MHz configurations (exact integer mode)
    {19200000, 8000, 0x01, 0x03, 0, 0},     // 19.2MHz, 8kHz, PSCLK=01, FREQ=11, 16KHZ=0
    {19200000, 16000, 0x01, 0x03, 1, 0},    // 19.2MHz, 16kHz, PSCLK=01, FREQ=11, 16KHZ=1
    {19200000, 24000, 0x01, 0x00, 0, 3200}, // 19.2MHz, 24kHz, normal mode
    {19200000, 32000, 0x01, 0x00, 0, 2400}, // 19.2MHz, 32kHz, normal mode
    {19200000, 44100, 0x01, 0x00, 0, 1741}, // 19.2MHz, 44.1kHz, normal mode
    {19200000, 48000, 0x01, 0x00, 0, 1600}, // 19.2MHz, 48kHz, normal mode
    
    // Higher MCLK frequencies with prescaling
    {24000000, 8000, 0x02, 0x01, 0, 0},     // 24MHz, 8kHz, PSCLK=10, FREQ=01
    {24000000, 16000, 0x02, 0x01, 1, 0},    // 24MHz, 16kHz, PSCLK=10, FREQ=01
    {24000000, 48000, 0x02, 0x00, 0, 2000}, // 24MHz, 48kHz, PSCLK=10, normal mode
    
    {26000000, 8000, 0x02, 0x02, 0, 0},     // 26MHz, 8kHz, PSCLK=10, FREQ=10
    {26000000, 16000, 0x02, 0x02, 1, 0},    // 26MHz, 16kHz, PSCLK=10, FREQ=10
    
    // MCLK frequencies requiring prescaling by 4 (>40MHz)
    {48000000, 8000, 0x03, 0x01, 0, 0},     // 48MHz, 8kHz, PSCLK=11, FREQ=01
    {48000000, 16000, 0x03, 0x01, 1, 0},    // 48MHz, 16kHz, PSCLK=11, FREQ=01
    {48000000, 48000, 0x03, 0x00, 0, 4000}, // 48MHz, 48kHz, PSCLK=11, normal mode
};

// Digital Filter Configurations
enum MAX9860DigitalFilter {
  MAX9860_FILTER_DISABLED = 0x0,
  MAX9860_FILTER_ELLIPTICAL_16KHZ = 0x1,  // Elliptical highpass with 217Hz notch for 16kHz
  MAX9860_FILTER_BUTTERWORTH_16KHZ = 0x2, // 500Hz Butterworth highpass for 16kHz
  MAX9860_FILTER_ELLIPTICAL_8KHZ = 0x3,   // Elliptical highpass with 217Hz notch for 8kHz
  MAX9860_FILTER_BUTTERWORTH_8KHZ = 0x4,  // 500Hz Butterworth highpass for 8kHz
  MAX9860_FILTER_BUTTERWORTH_48KHZ = 0x5, // 200Hz Butterworth highpass for 48kHz
};

}  // namespace max9860
}  // namespace esphome