#pragma once

#define USE_IPIXELCOMMANDS

#include <esp_gattc_api.h>
#include <algorithm>
#include <iterator>
#include <map>
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/base_light_effects.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/button/button.h"
#include "esphome/components/text/text.h"
#include "esphome/core/component.h"
#include "esp_heap_caps.h"
#include "helpers.h"
#include "state.h"

#ifdef HAS_PSRAM
#include "esphome/components/http_request/http_request_idf.h"
#define MAX_CHUNK_SIZE (12*1024)
#define MAX_READ_SIZE (32*1024)
#endif

namespace esphome {
namespace ipixel_ble {

class DeviceInfo {
public:
  DeviceInfo() = default;

  bool get_device_info(const std::vector<uint8_t> &res);
  void set_name(std::string &name) { name_ = name; }

  std::string name_{};
  std::string mcu_version_{};
  std::string wifi_verion_{};
  bool has_wifi_{false};
  bool password_flag_{false};
  uint8_t width_{0};
  uint8_t height_{0};

private:
  // known device types (byte 5 of notification buffer)
  std::map<const uint8_t, std::pair<uint8_t, uint8_t>> display_size_
  {
    {128, {64, 64}},
    {129, {32, 32}},
    {130, {32, 16}},
    {131, {64, 16}},
    {132, {96, 16}},
    {133, {64, 20}},
    {134, {128, 32}},
    {135, {144, 16}},
    {136, {192, 16}},
    {137, {48, 24}},
    {138, {64, 32}},
    {139, {96, 32}},
    {140, {128, 32}},
    {141, {96, 32}},
    {142, {160, 32}},
    {143, {192, 32}},
    {144, {256, 32}},
    {145, {320, 32}},
    {146, {384, 32}},
    {147, {448, 32}},
  };
};

#ifdef HAS_PSRAM
class ChunkBuffer {
public:
  ChunkBuffer() = default;

  bool ps_alloc(size_t buffer_size) {
    ptr_ = static_cast<uint8_t*> (heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM));
    if (ptr_ == nullptr) {
      size_ = 0;
    }
    return ptr_ != nullptr;
  }

  void ps_free() {
    init_();
  }

  uint8_t* get_ptr() {
    return ptr_;
  }

  const std::vector<uint8_t> get_buffer() {
    return std::vector<uint8_t>(ptr_, ptr_ + size_);
  }

  bool read_chunk() {
    if (read_ < size_ && downloader_ != nullptr) {
      size_t chunk_size = size_ - read_;
      if (chunk_size > MAX_READ_SIZE) chunk_size = MAX_READ_SIZE;
      if (ptr_ == nullptr && !ps_alloc(size_)) {
        return false;
      }
      downloader_->read(ptr_ + read_, chunk_size);
      size_t read_size = downloader_->get_bytes_read();
      if (read_size == 0) {
        ESP_LOGE("ChunkBuffer", "reading failed (request = %d got %d)", chunk_size, read_size);
        downloader_->end();
        ps_free();
        return false;
      }
      ESP_LOGI("ChunkBuffer", "read %d bytes", chunk_size);
      crc_ = crc32Update(ptr_ + read_, chunk_size, crc_);
      read_ += chunk_size;
      if (read_ >= size_) { // read finished
        crc_ = crc32Final(crc_);
        crc_str_ = Helpers::calculateCRC32Bytes(crc_);
        downloader_->end();
        ESP_LOGD("ChunkBuffer", "crc = 0x%02x%02x%02x%02x", crc_str_[0], crc_str_[1], crc_str_[2], crc_str_[3]);
      }
    }
    return read_ < size_;
  }

  const std::vector<uint8_t> get_chunk(uint8_t &index) {
    index = 0;
    if (pos_ < size_) {
      // first we have to read the entire buffer because the correct crc_str is required for the first sendImage command :(
      if (read_chunk()) return {};
      size_t chunk_size = size_ - pos_;
      if (chunk_size > MAX_CHUNK_SIZE) chunk_size = MAX_CHUNK_SIZE;
      ESP_LOGI("ChunkBuffer", "upload %d bytes)", chunk_size);
      index = chunk_index_;
      chunk_index_++;
      size_t pos = pos_;
      pos_ += chunk_size;
      return std::vector<uint8_t>(ptr_ + pos, ptr_ + pos + chunk_size);
    } else {
      ps_free();
    }
    return {};
  }

  void set_size(size_t size) { init_(); size_ = size; }
  size_t get_size() { return size_; }

  const std::vector<uint8_t> &get_crc() {
     return crc_str_;
  }

  bool is_valid() {
    return size_ != 0;
  }

  // will be nullptr after calling downloader_->end()
  std::shared_ptr<http_request::HttpContainer> downloader_{nullptr};

private:
  void init_() {
    if (ptr_ != nullptr) free(ptr_);
    ptr_ = nullptr;
    size_ = 0L;
    pos_ = 0L;
    read_ = 0L;
    chunk_index_ = 0;
    crc_ = CRC32_INITIAL;
    crc_str_.clear();
  }
  size_t size_{0};
  size_t read_{0};
  size_t pos_{0};
  uint8_t chunk_index_{0};
  uint32_t crc_{CRC32_INITIAL};
  std::vector<uint8_t> crc_str_{};
  uint8_t* ptr_{nullptr};
};

class IPixelBLE :  public display::DisplayBuffer, public light::LightOutput, public ble_client::BLEClientNode, Parented<http_request::HttpRequestComponent> {
#else
class IPixelBLE :  public display::DisplayBuffer, public light::LightOutput, public ble_client::BLEClientNode {
#endif
public:
  IPixelBLE() {}

  void setup() override;
  void loop() override;
  void show_image(int8_t page = -1) { load_image_effect(page); }
  void set_text(std::string &text) { state_.txt_ = text; text_effect(); }
  void set_effect(effects efect) { state_.effect_ = efect; }
  void set_color(std::string slot_csv);
  void set_background_color(std::string slot_csv);
  void set_program_list(std::string slot_csv);
  void del_program_list(std::string slot_csv);
  void load_image_url(std::string url);

  #ifdef HAS_PSRAM
  // http request
  void set_parent(http_request::HttpRequestComponent *parent) { http_request_ = parent; }
  #endif

  // display
  //void update() override;
  int get_width_internal() override { return state_.mDisplayWidth; }
  int get_height_internal() override { return state_.mDisplayHeight; }
  display::DisplayType get_display_type() override { return display::DISPLAY_TYPE_COLOR; }
  void update() {}

  // optional display parameters
  void set_display_width(uint16_t val) { state_.mDisplayWidth = val; }
  void set_display_height(uint16_t val) { state_.mDisplayHeight = val; }
  
  // ble client
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;
  bool ble_write_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle, uint8_t *data, uint16_t len);
  bool ble_read_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle);
  bool ble_register_for_notify(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda);
  void on_notification_received(const std::vector<uint8_t> &data);
  
  // light
  light::LightTraits get_traits() {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::RGB});
    return traits;
  }
  void write_state(light::LightState *state);

  // Sensor setters
  void set_power_state(sensor::Sensor *sensor) { power_state_ = sensor; }
  void set_connect_state(sensor::Sensor *sensor) { connect_state_ = sensor; }
  void set_display_width(sensor::Sensor *sensor) { display_width_ = sensor; }
  void set_display_height(sensor::Sensor *sensor) { display_height_ = sensor; }
  void set_font_width(sensor::Sensor *sensor) { font_width_ = sensor; }
  void set_font_height(sensor::Sensor *sensor) { font_height_ = sensor; }
  void set_rotation(sensor::Sensor *sensor) { rotation_ = sensor; }
  void set_fun_mode(sensor::Sensor *sensor) { fun_mode_ = sensor; }
  void set_program_slot(sensor::Sensor *sensor) { program_slot_ = sensor; }
  void set_upload_queue(sensor::Sensor *sensor) { upload_queue_ = sensor; }

  void set_update_time_button(button::Button *button) { update_time_button_ = button; }
  void on_update_time_button_press();

  // Number setters
  void set_clock_style_number(number::Number *number) { clock_style_number_ = number; }
  void on_clock_style_number(float value);

  void set_lambda_slot_number(number::Number *number) { lambda_slot_number_ = number; }
  void on_lambda_slot_number(float value);

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

protected:
  // Effects
  void text_effect();
  void time_date_effect();
  void fill_color_effect();
  void load_image_effect(int8_t page = -1);
  void load_gif_effect();
  void fill_rainbow_effect();
  void rhythm_animation_effect();
  void rhythm_levels_effect();
  void random_pixel_effect();
  void alarm_effect();

  float frand() { return (float) rand() / RAND_MAX; }

  void draw_absolute_pixel_internal(int x, int y, Color color) override;  // display
 
  void update_state_(const DeviceState &new_state);
  void do_update_();
 
  uint8_t get_slot(bool countdown = true);
 
  bool is_starting();

  // Command Queue
  bool queueTick();
  void queuePush(std::vector<uint8_t> command);

  // Downloader
  void downloadTick();

# ifdef HAS_PSRAM
  http_request::HttpRequestComponent *http_request_{nullptr};
  ChunkBuffer buffer_;
# endif

  uint16_t handle_{0};
  esp32_ble_tracker::ESPBTUUID service_uuid_ = esp32_ble_tracker::ESPBTUUID::from_raw("000000fa-0000-1000-8000-00805f9b34fb");
  esp32_ble_tracker::ESPBTUUID characteristic_uuid_ = esp32_ble_tracker::ESPBTUUID::from_raw("0000fa02-0000-1000-8000-00805f9b34fb");
  esp32_ble_tracker::ESPBTUUID notify_uuid_ = esp32_ble_tracker::ESPBTUUID::from_raw("0000fa03-0000-1000-8000-00805f9b34fb");  

  DeviceInfo  device_info_;
  DeviceState state_;
  std::vector<std::vector<uint8_t>> queue_;
  bool is_ready_{false};
  
  uint32_t last_request_{0};
  uint32_t last_update_{0};
  uint32_t last_animation_{0};
  
  //const char component_source_;

  // Sensor fields
  sensor::Sensor *power_state_{};
  sensor::Sensor *connect_state_{};
  sensor::Sensor *display_width_{};
  sensor::Sensor *display_height_{};
  sensor::Sensor *font_width_{};
  sensor::Sensor *font_height_{};
  sensor::Sensor *rotation_{};
  sensor::Sensor *fun_mode_{};
  sensor::Sensor *program_slot_{};
  sensor::Sensor *upload_queue_{};

  button::Button *update_time_button_{};

  number::Number *annimation_mode_number_{};
  number::Number *annimation_speed_number_{};
  number::Number *clock_style_number_{};
  number::Number *font_flag_number_{};
  number::Number *lambda_slot_number_{};
  number::Number *text_mode_number_{};

  switch_::Switch *play_switch_{};
};

}  // namespace ipixel_ble
}  // namespace esphome

