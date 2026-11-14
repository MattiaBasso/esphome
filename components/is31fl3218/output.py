import esphome.codegen as cg
from esphome.components import output
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_ID
from esphome.types import ConfigType

from . import CONF_IS31FL3218_ID, IS31FL3218, IS31FL3218_NUM_CHANNELS, is31fl3218_ns

DEPENDENCIES = ["is31fl3218"]

IS31FL3218Channel = is31fl3218_ns.class_("IS31FL3218Channel", output.FloatOutput)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(IS31FL3218Channel),
        cv.GenerateID(CONF_IS31FL3218_ID): cv.use_id(IS31FL3218),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=IS31FL3218_NUM_CHANNELS - 1),
    }
)


async def to_code(config: ConfigType) -> None:
    parent = await cg.get_variable(config[CONF_IS31FL3218_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(parent.register_channel(var))
    await output.register_output(var, config)
