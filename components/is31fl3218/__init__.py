"""ISSI IS31FL3218: 18-channel 8-bit PWM LED driver (I2C, fixed address 0x54).

Hub component. Platform: ``light`` (addressable RGB pixels made of three channels each, see ``light/``).
"""

from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ENABLE_PIN, CONF_ID
from esphome.types import ConfigType

CODEOWNERS = ["@MattiaBasso"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

is31fl3218_ns = cg.esphome_ns.namespace("is31fl3218")
IS31FL3218 = is31fl3218_ns.class_("IS31FL3218", cg.Component, i2c.I2CDevice)
CONF_IS31FL3218_ID = "is31fl3218_id"
IS31FL3218_NUM_CHANNELS = 18

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(IS31FL3218),
            # Optional SDB pin (hardware shutdown, active low): driven high in setup
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x54))
)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_ENABLE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(pin))
