import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv

from . import CONF_IPIXEL_BLE, IPixelBLE, ipixel_ble_ns

CONF_DELETE_SLOT = "delete_slot"
CONF_UPDATE_TIME = "update_time"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
        cv.Optional(CONF_DELETE_SLOT): button.button_schema(
            ipixel_ble_ns.class_("DeleteSlotButton", button.Button),
            icon="mdi:trash-can-outline",
        ),
        cv.Optional(CONF_UPDATE_TIME): button.button_schema(
            ipixel_ble_ns.class_("UpdateTimeButton", button.Button),
            icon="mdi:update",
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_IPIXEL_BLE])

    for var in [
        CONF_DELETE_SLOT,
        CONF_UPDATE_TIME
    ]:
        if conf := config.get(var):
            sw_var = await button.new_button(conf)
            await cg.register_parented(sw_var, parent)
            cg.add(getattr(parent, f"set_{var}_button")(sw_var))
