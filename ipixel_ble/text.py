import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text

from . import CONF_IPIXEL_BLE, IPixelBLE, ipixel_ble_ns

CONFIG_SCHEMA = text.TEXT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_IPIXEL_BLE])
    await text.register_text(parent, config)
