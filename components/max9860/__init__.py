import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ['i2c']

max9860_ns = cg.esphome_ns.namespace('max9860')
MAX9860Component = max9860_ns.class_('MAX9860Component', cg.Component, i2c.I2CDevice)

# Add a volume configuration option
CONF_STARTUP_VOLUME = "startup_volume"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MAX9860Component),
    # Default to 0x06 (-6dB). Allow user to set 0x00 for Max Volume.
    cv.Optional(CONF_STARTUP_VOLUME, default=0x06): cv.hex_uint8_t,
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x10))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    # Pass the volume setting to C++
    cg.add(var.set_startup_volume(config[CONF_STARTUP_VOLUME]))