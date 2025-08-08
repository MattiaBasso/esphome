import esphome.codegen as cg
from esphome.components import i2c
from esphome.components.audio_dac import AudioDac
import esphome.config_validation as cv
from esphome.const import CONF_BITS_PER_SAMPLE, CONF_ID, CONF_MIC_GAIN, CONF_SAMPLE_RATE

CODEOWNERS = ["@username"]  # Replace with your GitHub username
DEPENDENCIES = ["i2c"]

max9860_ns = cg.esphome_ns.namespace("max9860")
MAX9860 = max9860_ns.class_("MAX9860", AudioDac, cg.Component, i2c.I2CDevice)

CONF_USE_MICROPHONE = "use_microphone"
CONF_MASTER_MODE = "master_mode"
CONF_PLL_MODE = "pll_mode"
CONF_LEFT_ADC_ENABLE = "left_adc_enable"
CONF_RIGHT_ADC_ENABLE = "right_adc_enable"
CONF_AGC_ENABLE = "agc_enable"
CONF_SIDETONE_GAIN = "sidetone_gain"

max9860_mic_preamp_gain = max9860_ns.enum("MAX9860MicPreampGain")
MAX9860_MIC_PREAMP_GAIN_ENUM = {
    "OFF": max9860_mic_preamp_gain.MAX9860_MIC_PREAMP_OFF,
    "0DB": max9860_mic_preamp_gain.MAX9860_MIC_PREAMP_0DB,
    "20DB": max9860_mic_preamp_gain.MAX9860_MIC_PREAMP_20DB,
    "30DB": max9860_mic_preamp_gain.MAX9860_MIC_PREAMP_30DB,
}

max9860_mic_pga_gain = max9860_ns.enum("MAX9860MicPGAGain")
MAX9860_MIC_PGA_GAIN_ENUM = {
    f"{i}DB": max9860_mic_pga_gain[f"MAX9860_MIC_PGA_{i}DB"] for i in range(21)
}

max9860_sidetone_gain = max9860_ns.enum("MAX9860SidetoneGain")
MAX9860_SIDETONE_GAIN_ENUM = {
    "DISABLED": max9860_sidetone_gain.MAX9860_SIDETONE_DISABLED,
    "0DB": max9860_sidetone_gain.MAX9860_SIDETONE_0DB,
    **{f"-{i*2}DB": max9860_sidetone_gain[f"MAX9860_SIDETONE_MINUS_{i*2}DB"] for i in range(1, 31)},
}

_validate_bits = cv.float_with_unit("bits", "bit")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MAX9860),
            cv.Optional(CONF_BITS_PER_SAMPLE, default="16bit"): cv.All(
                _validate_bits, cv.one_of(16.0)
            ),  # MAX9860 only supports 16-bit
            cv.Optional(CONF_MIC_GAIN, default="0DB"): cv.enum(
                MAX9860_MIC_PGA_GAIN_ENUM, upper=True
            ),
            cv.Optional(CONF_SAMPLE_RATE, default=8000): cv.int_range(min=8000, max=48000),
            cv.Optional(CONF_USE_MICROPHONE, default=False): cv.boolean,
            cv.Optional(CONF_MASTER_MODE, default=True): cv.boolean,
            cv.Optional(CONF_PLL_MODE, default=False): cv.boolean,
            cv.Optional(CONF_LEFT_ADC_ENABLE, default=True): cv.boolean,
            cv.Optional(CONF_RIGHT_ADC_ENABLE, default=False): cv.boolean,
            cv.Optional(CONF_AGC_ENABLE, default=False): cv.boolean,
            cv.Optional(CONF_SIDETONE_GAIN, default="DISABLED"): cv.enum(
                MAX9860_SIDETONE_GAIN_ENUM, upper=True
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x10))  # MAX9860 I2C address is 0x10 (7-bit)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_sample_frequency(config[CONF_SAMPLE_RATE]))
    cg.add(var.set_mic_pga_gain(config[CONF_MIC_GAIN]))
    cg.add(var.set_use_microphone(config[CONF_USE_MICROPHONE]))
    cg.add(var.set_master_mode(config[CONF_MASTER_MODE]))
    cg.add(var.set_pll_mode(config[CONF_PLL_MODE]))
    cg.add(var.set_left_adc_enable(config[CONF_LEFT_ADC_ENABLE]))
    cg.add(var.set_right_adc_enable(config[CONF_RIGHT_ADC_ENABLE]))
    cg.add(var.set_agc_enable(config[CONF_AGC_ENABLE]))
    cg.add(var.set_sidetone_gain(config[CONF_SIDETONE_GAIN]))
