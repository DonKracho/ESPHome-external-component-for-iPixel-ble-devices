import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display

from . import CONF_IPIXEL_BLE, IPixelBLE, ipixel_ble_ns

DEPENDENCIES = ["ipixel_ble"]

CONF_WIDTH = "width"
CONF_HEIGHT = "height"

CONFIG_SCHEMA = display.FULL_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
        cv.Optional(CONF_WIDTH,  default = 32): cv.All(cv.uint16_t, cv.Range(min = 16, max = 128)),
        cv.Optional(CONF_HEIGHT, default = 32): cv.All(cv.uint16_t, cv.Range(min = 16, max = 128)),
    }
)


async def to_code(config):
    var = await cg.get_variable(config[CONF_IPIXEL_BLE])

    # Set custom variables. Due to default values no need for availability
    cg.add(var.set_display_width(config[CONF_WIDTH])),
    cg.add(var.set_display_height(config[CONF_HEIGHT])),
