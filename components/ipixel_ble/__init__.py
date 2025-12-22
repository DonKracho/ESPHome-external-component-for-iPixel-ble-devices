import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, light, ble_client
from esphome.components.http_request import CONF_HTTP_REQUEST_ID, HttpRequestComponent
from esphome.const import CONF_ID
#from esphome.const import __version__ as ESPHOME_VERSION


CODEOWNERS = ["@donkracho"]
#DEPENDENCIES = ["http_request"]

ESPHOME_MIN_VERSION = "2025.11.0"
CONF_IPIXEL_BLE = "ipixel_ble"

ipixel_ble_ns = cg.esphome_ns.namespace("ipixel_ble")
IPixelBLE = ipixel_ble_ns.class_("IPixelBLE", cg.Component, display.DisplayBuffer, light.LightOutput, ble_client.BLEClientNode)

CONFIG_ESPC3 = cv.Schema({
    cv.GenerateID(): cv.declare_id(IPixelBLE)
})

CONFIG_ESPS3_PSRAM = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(IPixelBLE),
    cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent)
})

CONFIG_SCHEMA = (
    CONFIG_ESPC3
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend({cv.Optional(""): cv.validate_esphome_version(ESPHOME_MIN_VERSION)})
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
    if http := config.get(CONF_HTTP_REQUEST_ID):
      await cg.register_parented(var, config[http])
