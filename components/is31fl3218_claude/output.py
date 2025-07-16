import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_CHANNEL, CONF_ID
from .output import IS31FL3218Component, IS31FL3218Channel, is31fl3218_ns

DEPENDENCIES = ["is31fl3218"]

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(IS31FL3218Channel),
    cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=18),
    cv.Required("is31fl3218_id"): cv.use_id(IS31FL3218Component),
})

async def to_code(config):
    parent = await cg.get_variable(config["is31fl3218_id"])
    channel = config[CONF_CHANNEL] - 1  # Convert to 0-based indexing
    var = cg.new_Pvariable(config[CONF_ID], parent, channel)
    await output.register_output(var, config)
    return var