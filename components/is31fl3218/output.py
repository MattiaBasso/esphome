import esphome.codegen as cg
from esphome.components import output
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_ID

from . import IS31FL3218Output, is31fl3218_ns

DEPENDENCIES = ["is31fl3218"]

IS31FL3218Channel = is31fl3218_ns.class_("IS31FL3218Channel", output.FloatOutput)
CONF_IS31FL3218_ID = "is31fl3218_id"

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(IS31FL3218Channel),
        cv.GenerateID(CONF_IS31FL3218_ID): cv.use_id(IS31FL3218Output),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=17),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_IS31FL3218_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(paren.register_channel(var))
    await output.register_output(var, config)