import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from . import CONF_IPIXEL_BLE, IPixelBLE, ipixel_ble_ns

# Switch configurations
CONF_PLAY = "play"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
        cv.Optional(CONF_PLAY): switch.switch_schema(
            ipixel_ble_ns.class_("PlaySwitch", switch.Switch),
            icon="mdi:play",
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_IPIXEL_BLE])

    for switch_type in [
      CONF_PLAY
    ]:
        if conf := config.get(switch_type):
            sw_var = await switch.new_switch(conf)
            await cg.register_parented(sw_var, parent)
            cg.add(getattr(parent, f"set_{switch_type}_switch")(sw_var))
