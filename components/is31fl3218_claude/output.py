import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
MULTI_CONF = True

is31fl3218_ns = cg.esphome_ns.namespace("empty_float_output")
IS31FL3218 = is31fl3218_ns.class_(
    "IS31FL3218", output.FloatOutput, cg.Component
)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(IS31FL3218),
    }
).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x54))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await i2c.register_i2c_device(var, config)
    await cg.register_component(var, config)