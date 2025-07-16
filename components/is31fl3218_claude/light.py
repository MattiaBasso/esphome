# light.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_CHANNEL, CONF_ID, CONF_OUTPUT_ID

from . import IS31FL3218Component, IS31FL3218Channel, is31fl3218_ns, CONF_IS31FL3218_ID

DEPENDENCIES = ["is31fl3218"]

CONFIG_SCHEMA = light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(IS31FL3218Channel),
        cv.GenerateID(CONF_IS31FL3218_ID): cv.use_id(IS31FL3218Component),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=17),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_IS31FL3218_ID])
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], parent, config[CONF_CHANNEL])
    await light.register_light(var, config)