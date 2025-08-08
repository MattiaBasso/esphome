# MAX9860 Audio Codec ESPHome Component

This is an external ESPHome component for the MAX9860 16-bit mono audio codec. The MAX9860 is a low-power audio codec designed for voice applications, headsets, and mobile devices.

## Features

- 16-bit mono audio codec
- Sample rates from 8kHz to 48kHz  
- Built-in headphone amplifier (30mW into 32Ω)
- Dual differential microphone inputs
- Automatic Gain Control (AGC) and Noise Gate
- Digital filtering with GSM-compliant highpass filters
- Master or slave I2S operation
- Low power consumption (suitable for battery applications)

## Hardware Connections

The MAX9860 uses I2C for control and I2S for audio data:

### I2C Interface
- **SDA**: I2C data line
- **SCL**: I2C clock line  
- **Address**: 0x10 (7-bit address)

### I2S Audio Interface
- **MCLK**: Master clock input
- **LRCLK**: Left/Right clock (frame sync)
- **BCLK**: Bit clock
- **SDIN**: Serial data input (to DAC)
- **SDOUT**: Serial data output (from ADC)

### Audio I/O
- **OUTP/OUTN**: Differential headphone outputs
- **MICLP/MICLN**: Left differential microphone input
- **MICRP/MICRN**: Right differential microphone input
- **MICBIAS**: Microphone bias output (1.55V)

### Power Supply
- **AVDD/DVDD**: 1.8V analog/digital supply
- **DVDDIO**: 1.8V to 3.6V I/O supply

## Example Configuration

### Basic Audio Output Configuration

```yaml
# Enable I2C
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  id: bus_a

# Enable I2S audio
i2s_audio:
  i2s_lrclk_pin: GPIO25
  i2s_bclk_pin: GPIO26

# Configure MAX9860 codec
max9860:
  - platform: max9860
    id: audio_codec
    i2c_id: bus_a
    address: 0x10
    sample_rate: 16000
    master_mode: true
    use_microphone: false

# Media player using the codec
media_player:
  - platform: i2s_audio
    id: media_out
    name: "Audio Player"
    dac_type: external
    i2s_dout_pin: GPIO27
    audio_dac: audio_codec
```

### Full-Duplex Configuration with Microphone

```yaml
# Enable I2C
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

# Enable I2S audio  
i2s_audio:
  i2s_lrclk_pin: GPIO25
  i2s_bclk_pin: GPIO26

# Configure MAX9860 with microphone support
max9860:
  - platform: max9860
    id: audio_codec
    sample_rate: 8000
    master_mode: true
    use_microphone: true
    left_adc_enable: true
    right_adc_enable: false
    agc_enable: true
    mic_gain: "10DB"
    sidetone_gain: "-20DB"

# Media player for audio output
media_player:
  - platform: i2s_audio
    id: media_out
    name: "Audio Player"
    dac_type: external
    i2s_dout_pin: GPIO27
    audio_dac: audio_codec

# Microphone for audio input
microphone:
  - platform: i2s_audio
    id: mic_input
    name: "Audio Input"
    i2s_din_pin: GPIO35
    adc_type: external
    pdm: false
```

## Configuration Variables

### Required
- **sample_rate** (*int*): Audio sample rate in Hz. Supported rates: 8000, 16000, 24000, 32000, 44100, 48000. Default: `8000`.

### Optional
- **master_mode** (*boolean*): Set to `true` for I2S master mode, `false` for slave mode. Default: `true`.
- **use_microphone** (*boolean*): Enable microphone input. Default: `false`.
- **left_adc_enable** (*boolean*): Enable left ADC channel. Default: `true`.
- **right_adc_enable** (*boolean*): Enable right ADC channel. Default: `false`.
- **pll_mode** (*boolean*): Enable PLL for slave mode operation. Default: `false`.
- **agc_enable** (*boolean*): Enable Automatic Gain Control for microphones. Default: `false`.
- **mic_gain** (*enum*): Microphone PGA gain. Options: `0DB` to `20DB` (1dB steps). Default: `0DB`.
- **sidetone_gain** (*enum*): Sidetone mixing level. Options: `DISABLED`, `0DB`, `-2DB` to `-60DB` (2dB steps). Default: `DISABLED`.

## Supported MCLK Frequencies

The component automatically detects and configures appropriate clock settings for these MCLK frequencies:
- 12 MHz (exact integer mode for 8/16 kHz)
- 13 MHz (exact integer mode for 8/16 kHz)  
- 19.2 MHz (exact integer mode for 8/16 kHz)
- 24 MHz
- 26 MHz
- 48 MHz

## Installation

1. Create a `components/max9860/` directory in your ESPHome configuration folder
2. Copy all the component files to this directory:
   - `__init__.py` (empty file)
   - `audio_dac.py` (Python configuration)
   - `max9860.h` (C++ header)
   - `max9860.cpp` (C++ implementation)
   - `max9860_const.h` (Constants header)

3. Use the component in your YAML configuration as shown in the examples above.

## Notes

- The MAX9860 is a mono codec, so stereo input signals are mixed to mono
- For voice applications, 8kHz or 16kHz sample rates are recommended
- AGC and noise gate are useful for dynamic microphone environments
- The codec includes click-and-pop suppression for smooth volume changes
- Power consumption is optimized - unused ADC/DAC sections can be disabled

## Troubleshooting

1. **I2C Communication Issues**: Verify wiring and that the device appears at address 0x10 with `i2c: scan: true`
2. **No Audio Output**: Check I2S connections and ensure `master_mode` matches your system setup
3. **Poor Audio Quality**: Verify MCLK frequency is supported and stable
4. **Microphone Issues**: Enable `agc_enable` for better dynamic range, check `mic_gain` settings

## References

- [MAX9860 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX9860.pdf)
- [ESPHome I2S Audio Documentation](https://esphome.io/components/i2s_audio.html)
- [ESPHome Custom Components](https://esphome.io/custom/custom_component.html)