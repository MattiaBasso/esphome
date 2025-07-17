import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
MULTI_CONF = True

is31fl3218component_ns = cg.esphome_ns.namespace("is31fl3218")
IS31FL3218Component = is31fl3218component_ns.class_("IS31FL3218Component", cg.Component, i2c.I2CDevice)

MULTI_CONF = True
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(IS31FL3218Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x54))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)