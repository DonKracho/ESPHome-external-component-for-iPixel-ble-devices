import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv

from . import CONF_IPIXEL_BLE, IPixelBLE, ipixel_ble_ns

AUTO_LOAD = ["number"]

CONF_CLOCK_STYLE = "clock_style"
ClockStyleNumber = ipixel_ble_ns.class_("ClockStyleNumber", number.Number)

CONF_SLOT_NUMBER = "slot_number"
SlotNumberNumber = ipixel_ble_ns.class_("SlotNumberNumber", number.Number)

CONF_ANNIMATION_MODE = "annimation_mode"
AnnimationModeNumber = ipixel_ble_ns.class_("AnnimationModeNumber", number.Number)

CONF_ANNIMATION_SPEED = "annimation_speed"
AnnimationSpeedNumber = ipixel_ble_ns.class_("AnnimationSpeedNumber", number.Number)

CONF_FONT_FLAG = "font_flag"
FontFlagNumber = ipixel_ble_ns.class_("FontFlagNumber", number.Number)

CONF_TEXT_MODE = "text_mode"
TextModeNumber = ipixel_ble_ns.class_("TextModeNumber", number.Number)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
        cv.Optional(CONF_CLOCK_STYLE): number.number_schema(ClockStyleNumber),
        cv.Optional(CONF_SLOT_NUMBER): number.number_schema(SlotNumberNumber),
        cv.Optional(CONF_ANNIMATION_MODE): number.number_schema(AnnimationModeNumber),
        cv.Optional(CONF_ANNIMATION_SPEED): number.number_schema(AnnimationSpeedNumber),
        cv.Optional(CONF_FONT_FLAG): number.number_schema(FontFlagNumber),
        cv.Optional(CONF_TEXT_MODE): number.number_schema(TextModeNumber),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_IPIXEL_BLE])

    # CONF_POWER_LEVEL
    if conf := config.get(CONF_CLOCK_STYLE):
        b = await number.new_number(conf, min_value=1, max_value=9, step=1)
        await cg.register_parented(b, parent)
        cg.add(getattr(parent, f"set_{CONF_CLOCK_STYLE}_number")(b))
    if conf := config.get(CONF_SLOT_NUMBER):
        b = await number.new_number(conf, min_value=1, max_value=10, step=1)
        await cg.register_parented(b, parent)
        cg.add(getattr(parent, f"set_{CONF_SLOT_NUMBER}_number")(b))
    if conf := config.get(CONF_ANNIMATION_MODE):
        b = await number.new_number(conf, min_value=0, max_value=6, step=1)
        await cg.register_parented(b, parent)
        cg.add(getattr(parent, f"set_{CONF_ANNIMATION_MODE}_number")(b))
    if conf := config.get(CONF_ANNIMATION_SPEED):
        b = await number.new_number(conf, min_value=0, max_value=100, step=5)
        await cg.register_parented(b, parent)
        cg.add(getattr(parent, f"set_{CONF_ANNIMATION_SPEED}_number")(b))
    if conf := config.get(CONF_FONT_FLAG):
        b = await number.new_number(conf, min_value=0, max_value=4, step=1)
        await cg.register_parented(b, parent)
        cg.add(getattr(parent, f"set_{CONF_FONT_FLAG}_number")(b))
    if conf := config.get(CONF_TEXT_MODE):
        b = await number.new_number(conf, min_value=0, max_value=4, step=1)
        await cg.register_parented(b, parent)
        cg.add(getattr(parent, f"set_{CONF_TEXT_MODE}_number")(b))
