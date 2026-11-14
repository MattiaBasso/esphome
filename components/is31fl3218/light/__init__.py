"""IS31FL3218 as an addressable light: each pixel is an RGB triplet of driver channels.

Effects (addressable_lambda, addressable_twinkle, ...) and partitions work natively.
"""

import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_OUTPUT_ID, CONF_RED
from esphome.types import ConfigType

from .. import CONF_IS31FL3218_ID, IS31FL3218, IS31FL3218_NUM_CHANNELS, is31fl3218_ns

DEPENDENCIES = ["is31fl3218"]

IS31FL3218Light = is31fl3218_ns.class_(
    "IS31FL3218Light", light.AddressableLight, cg.Parented.template(IS31FL3218)
)

CONF_LEDS = "leds"

CHANNEL = cv.int_range(min=0, max=IS31FL3218_NUM_CHANNELS - 1)
LED_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_RED): CHANNEL,
        cv.Required(CONF_GREEN): CHANNEL,
        cv.Required(CONF_BLUE): CHANNEL,
    }
)


def _unique_channels(leds):
    used = [led[key] for led in leds for key in (CONF_RED, CONF_GREEN, CONF_BLUE)]
    if len(used) != len(set(used)):
        raise cv.Invalid("Each channel can be used only once in a light")
    return leds


CONFIG_SCHEMA = light.light_schema(IS31FL3218Light, light.LightType.ADDRESSABLE).extend(
    {
        cv.GenerateID(CONF_IS31FL3218_ID): cv.use_id(IS31FL3218),
        # Pixels in order (index 0 first). Channel n = pin OUT(n+1).
        cv.Required(CONF_LEDS): cv.All(
            cv.ensure_list(LED_SCHEMA),
            cv.Length(min=1, max=IS31FL3218_NUM_CHANNELS // 3),
            _unique_channels,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await light.register_light(var, config)
    await cg.register_parented(var, config[CONF_IS31FL3218_ID])
    for led in config[CONF_LEDS]:
        cg.add(var.add_led(led[CONF_RED], led[CONF_GREEN], led[CONF_BLUE]))
