from esphome import automation, pins
import esphome.codegen as cg
from esphome.components import i2c
from esphome.components.audio_dac import AudioDac
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_SAMPLE_RATE, CONF_TRIGGER_ID
from esphome.types import ConfigType

CODEOWNERS = ["@MattiaBasso"]
DEPENDENCIES = ["i2c"]

max9860_ns = cg.esphome_ns.namespace("max9860")
MAX9860 = max9860_ns.class_("MAX9860", AudioDac, cg.Component, i2c.I2CDevice)
ClipTrigger = max9860_ns.class_("ClipTrigger", automation.Trigger.template())
PllUnlockTrigger = max9860_ns.class_("PllUnlockTrigger", automation.Trigger.template())

CONF_MCLK_FREQUENCY = "mclk_frequency"
CONF_USE_MICROPHONE = "use_microphone"
CONF_MIC_PREAMP_GAIN = "mic_preamp_gain"
CONF_MIC_PGA_GAIN = "mic_pga_gain"
CONF_AGC_TARGET = "agc_target"
CONF_MIN_VOLUME = "min_volume"
CONF_IRQ_PIN = "irq_pin"
CONF_ON_CLIP = "on_clip"
CONF_ON_PLL_UNLOCK = "on_pll_unlock"

max9860_preamp_gain = max9860_ns.enum("MAX9860PreampGain")
MAX9860_PREAMP_GAIN_ENUM = {
    "0DB": max9860_preamp_gain.MAX9860_PREAMP_GAIN_0DB,
    "20DB": max9860_preamp_gain.MAX9860_PREAMP_GAIN_20DB,
    "30DB": max9860_preamp_gain.MAX9860_PREAMP_GAIN_30DB,
}


def _validate_mclk(value):
    # Datasheet: MCLK 10-60 MHz in every clocking mode
    value = cv.frequency(value)
    if not 10e6 <= value <= 60e6:
        raise cv.Invalid("MAX9860 MCLK must be between 10MHz and 60MHz")
    return value


def _validate_irq(config):
    for key in (CONF_ON_CLIP, CONF_ON_PLL_UNLOCK):
        if key in config and CONF_IRQ_PIN not in config:
            raise cv.Invalid(f"{key} requires {CONF_IRQ_PIN}")
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MAX9860),
            cv.Optional(CONF_MCLK_FREQUENCY, default="12.288MHz"): _validate_mclk,
            cv.Optional(CONF_SAMPLE_RATE, default=16000): cv.int_range(
                min=8000, max=48000
            ),
            cv.Optional(CONF_USE_MICROPHONE, default=True): cv.boolean,
            cv.Optional(CONF_MIC_PREAMP_GAIN, default="20DB"): cv.enum(
                MAX9860_PREAMP_GAIN_ENUM, upper=True
            ),
            cv.Optional(CONF_MIC_PGA_GAIN, default="0dB"): cv.All(
                cv.decibel, cv.int_range(min=0, max=20)
            ),
            # Presence enables the microphone AGC (the PGA is then driven by the AGC)
            cv.Optional(CONF_AGC_TARGET): cv.All(
                cv.decibel, cv.int_range(min=-18, max=-3)
            ),
            cv.Optional(CONF_MIN_VOLUME, default="-50dB"): cv.All(
                cv.decibel, cv.int_range(min=-90, max=-6)
            ),
            # IRQ output (open drain, active low): clip detect + digital PLL unlock interrupts
            cv.Optional(CONF_IRQ_PIN): pins.internal_gpio_input_pin_schema,
            # Clipping in the ADC/DAC (or mic AGC at 0 dB); at most once per second
            cv.Optional(CONF_ON_CLIP): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ClipTrigger)}
            ),
            # Digital PLL unlocked after it had locked once (audio data not reliable)
            cv.Optional(CONF_ON_PLL_UNLOCK): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PllUnlockTrigger)}
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x10)),
    _validate_irq,
)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_mclk_frequency(int(config[CONF_MCLK_FREQUENCY])))
    cg.add(var.set_sample_rate(config[CONF_SAMPLE_RATE]))
    cg.add(var.set_use_microphone(config[CONF_USE_MICROPHONE]))
    cg.add(var.set_mic_preamp_gain(config[CONF_MIC_PREAMP_GAIN]))
    cg.add(var.set_mic_pga_gain(config[CONF_MIC_PGA_GAIN]))
    if CONF_AGC_TARGET in config:
        cg.add(var.set_agc_target(config[CONF_AGC_TARGET]))
    cg.add(var.set_min_volume_db(config[CONF_MIN_VOLUME]))
    if irq_pin := config.get(CONF_IRQ_PIN):
        cg.add(var.set_irq_pin(await cg.gpio_pin_expression(irq_pin)))
    for conf in config.get(CONF_ON_CLIP, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_PLL_UNLOCK, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
