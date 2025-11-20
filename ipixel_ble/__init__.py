import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, sensor
from esphome.const import CONF_ID

CODEOWNERS = ["@donkracho"]
DEPENDENCIES = ["ble_client", "output"]

CONF_IPIXEL_BLE = "ipixel_ble"

ipixel_ble_ns = cg.esphome_ns.namespace("ipixel_ble")
IPixelBLE = ipixel_ble_ns.class_("IPixelBLE", sensor.Sensor, cg.Component, ble_client.BLEClientNode)

CONFIG_SCHEMA = (
    cv.Schema({cv.GenerateID(): cv.declare_id(IPixelBLE)})
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ble_client.BLE_CLIENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
