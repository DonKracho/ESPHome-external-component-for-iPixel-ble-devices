import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
)

from . import CONF_IPIXEL_BLE, IPixelBLE

# Sensor configurations
CONF_CONNECT_STATE = "connect_state"
CONF_DISPLAY_WIDTH = "display_width"
CONF_DISPLAY_HEIGHT = "display_height"
CONF_FONT_FLAG = "font_flag"
CONF_FONT_WIDTH = "font_width"
CONF_FONT_HEIGHT = "font_height"
CONF_ROTATION = "rotation"
CONF_FUN_MODE = "fun_mode"
CONF_PROGRAM_SLOT = "program_slot"
CONF_UPLOAD_QUEUE = "upload_queue"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_IPIXEL_BLE): cv.use_id(IPixelBLE),
        cv.Optional(CONF_CONNECT_STATE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_DISPLAY_WIDTH): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_DISPLAY_HEIGHT): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_FONT_FLAG): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_FONT_WIDTH): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_FONT_HEIGHT): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ROTATION): sensor.sensor_schema(
            unit_of_measurement="°",
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_FUN_MODE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Required(CONF_PROGRAM_SLOT): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Required(CONF_UPLOAD_QUEUE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


async def to_code(config):
    var = await cg.get_variable(config[CONF_IPIXEL_BLE])

    for sensor_name in [
        CONF_CONNECT_STATE,
        CONF_DISPLAY_WIDTH,
        CONF_DISPLAY_HEIGHT,
        CONF_FONT_FLAG,
        CONF_FONT_WIDTH,
        CONF_FONT_HEIGHT,
        CONF_ROTATION,
        CONF_FUN_MODE,
        CONF_PROGRAM_SLOT,
        CONF_UPLOAD_QUEUE,
    ]:
        if sensor_config := config.get(sensor_name):
            sens = await sensor.new_sensor(sensor_config)
            cg.add(getattr(var, f"set_{sensor_name}")(sens))
