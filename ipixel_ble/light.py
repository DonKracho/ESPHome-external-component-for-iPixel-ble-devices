import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output

from . import CONF_IPIXEL_BLE, IPixelBLE, ipixel_ble_ns

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_IPIXEL_BLE])
    await light.register_light(parent, config)
