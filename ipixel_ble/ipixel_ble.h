#pragma once

#ifdef USE_ESP32

#define USE_IPIXELCOMMANDS

#include <esp_gattc_api.h>
#include <algorithm>
#include <iterator>
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/button/button.h"
#include "esphome/components/text/text.h"
#include "esphome/core/component.h"

#include "state.h"

namespace esphome {
namespace ipixel_ble {

class IPixelBLE : public Component, public ble_client::BLEClientNode, public light::LightOutput, public text::Text {
 public:
  IPixelBLE() = default;

  void loop() override;

  // ble client
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;

  bool ble_write_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle, uint8_t *data, uint16_t len);
  bool ble_read_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle);
  bool ble_register_for_notify(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda);

  void on_notification_received(const std::vector<uint8_t> &data);
  void update_sensors(const DeviceState &new_state);

  // light
  light::LightTraits get_traits() {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB});
    return traits;
  }
  void write_state(light::LightState *state);

  // text
  void control(const std::string &value);

  // Sensor setters
  void set_power_state(sensor::Sensor *sensor) { power_state_ = sensor; }
  void set_connect_state(sensor::Sensor *sensor) { connect_state_ = sensor; }
  void set_display_width(sensor::Sensor *sensor) { display_width_ = sensor; }
  void set_display_height(sensor::Sensor *sensor) { display_height_ = sensor; }
  void set_font_width(sensor::Sensor *sensor) { font_width_ = sensor; }
  void set_font_height(sensor::Sensor *sensor) { font_height_ = sensor; }
  void set_orientation(sensor::Sensor *sensor) { orientation_ = sensor; }
  void set_fun_mode(sensor::Sensor *sensor) { fun_mode_ = sensor; }

  // Button setters
  void set_delete_slot_button(button::Button *button) { delete_slot_button_ = button; }
  void on_delete_slot_button_press();

  void set_update_time_button(button::Button *button) { update_time_button_ = button; }
  void on_update_time_button_press();

  // Number setters
  void set_clock_style_number(number::Number *number) { clock_style_number_ = number; }
  void on_clock_style_number(float value);

  void set_slot_number_number(number::Number *number) { slot_number_number_ = number; }
  void on_slot_number_number(float value);

  void set_annimation_mode_number(number::Number *number) { annimation_mode_number_ = number; }
  void on_annimation_mode_number(float value);

  void set_annimation_speed_number(number::Number *number) { annimation_speed_number_ = number; }
  void on_annimation_speed_number(float value);

  void set_font_flag_number(number::Number *number) { font_flag_number_ = number; }
  void on_font_flag_number(float value);

  void set_text_mode_number(number::Number *number) { text_mode_number_ = number; }
  void on_text_mode_number(float value);

  // Switch setter
  void set_play_switch(switch_::Switch *sw) { play_switch_ = sw; }
  void on_play_switch(bool state);

  DeviceState get_state() { return this->state_; }

  // Queue
  void queueTick();
  void queuePush(std::vector<uint8_t> command);

  // Effects
  void text_effect();
  void time_date_effect();
  void fill_color_effect();
  void load_png_effect();
  void load_gif_effect();
  void fill_rainbow_effect();
  void random_pixel_effect();
  void alarm_effect();

  protected:
  uint16_t handle_{0};
  esp32_ble_tracker::ESPBTUUID service_uuid_ = esp32_ble_tracker::ESPBTUUID::from_raw("000000fa-0000-1000-8000-00805f9b34fb");
  esp32_ble_tracker::ESPBTUUID characteristic_uuid_ = esp32_ble_tracker::ESPBTUUID::from_raw("0000fa02-0000-1000-8000-00805f9b34fb");
                                              
  DeviceState state_;
  std::vector<std::vector<uint8_t>> queue;

  bool response_received_{false};
  uint32_t last_request_{0};
  uint32_t last_update_{0};
  uint32_t last_animation_{0};
  
  // Sensor fields
  sensor::Sensor *power_state_{};
  sensor::Sensor *connect_state_{};
  sensor::Sensor *display_width_{};
  sensor::Sensor *display_height_{};
  sensor::Sensor *font_width_{};
  sensor::Sensor *font_height_{};
  sensor::Sensor *orientation_{};
  sensor::Sensor *fun_mode_{};

  button::Button *delete_slot_button_{};
  button::Button *update_time_button_{};

  number::Number *clock_style_number_{};
  number::Number *slot_number_number_{};
  number::Number *annimation_mode_number_{};
  number::Number *annimation_speed_number_{};
  number::Number *font_flag_number_{};
  number::Number *text_mode_number_{};

  switch_::Switch *play_switch_{};
};

}  // namespace ipixel_ble
}  // namespace esphome

#endif  // USE_ESP32
