#include "ipixel_ble.h"
#include "iPixelCommands.h"
#include <cstdlib>  // rand
#include <cmath>    // sin
#include <algorithm>
#include <ctime>
#include <sstream>

#undef ESPHOME_LOG_LEVEL
#define ESPHOME_LOG_LEVEL(ESPHOME_LOG_LEVEL_DEBUG)
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace ipixel_ble {

const char *TAG = "ipixel_ble";

bool DeviceInfo::get_device_info(const std::vector<uint8_t> &res) {
  // gather informations from notify data as far as known
  // 0C.00.01.80.81.39.2C.00.0C.01.00.01 (example 32x32 notifcatoin)
  //
  // Sprache Engish
  // Gerätetyp 32x32
  // MCU-Firmware-versions 11.03
  // versions der Bluetooth Firmware: 7.02
  //
  // byte 0 length of notification in bytes (0x0C)
  // byte 1 repeated command byte
  // byte 2 repeated command byte
  // byte 3 repeated command byte
  // byte 4 encoded display size (has been hour before)
  // byte 5 repeated minutes
  // byte 6 repeated seconds
  // byte 7 repeated language
  // byte 8 variable value ?
  // byte 9 static 0x01 ?
  // byte 10 password flag ?
  // byte 11 static 0x01 ?

  if (res.size() < 6) {
    return false; // the standard cmd notify of 5 bytes length is out of focus here
  }

  if (res[2] == 0x05 && res[3] == 0x80) {
    ESP_LOGW(TAG, "device info: MCU fw=%d.%02d BLE fw=%d.%02d", res[4], res[5], res[6], res[7]);
    return false; // nice to know, but interested in the display size information only
  }
  
  if (res[2] == 0x01 && res[3] == 0x80) {
    auto const type = res[4];
    width_ =  display_size_[type].first;
    height_ =  display_size_[type].second;

    if (res.size() >= 11) {
      password_flag_ = res[10];
    }
    ESP_LOGW(TAG, "device info: name=%s size=%dx%d", name_.c_str(), width_, height_);
  }

  return width_ > 0 && height_ > 0;
;
}

// component
void IPixelBLE::setup() {
}

void IPixelBLE::loop() {
  if (this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED) {
    uint32_t tick = millis();
    if (this->last_request_ + (1000 *3600) < tick) {  // once per hour
      this->last_request_ = tick;
      on_update_time_button_press();  // trigger time update
    }
    
    if (state_.mPowerState) {
      if (this->last_animation_ + 300 < tick) {
        this->last_animation_ = tick;
        random_pixel_effect(); // update the animated effect whie it is selected
        alarm_effect();
      } 

      if (this->last_update_ + 5000 < tick) {
        this->last_update_ = tick;
        //load_gif_effect();  // loads a entire RGB frame, do not stress the BLE connection to much
      }
    }
    queueTick();
    downloadTick();
  }
  /*
  if (state_.mEffectRestore) {
    // restore last known effect
    if (state_.mEffectPtr != nullptr) {
      ESP_LOGW(TAG, "restoring effect"); 
      state_.mEffectPtr->apply();
    }
    state_.mEffectRestore = false;
  }
  */

  // update sensors and numbers even tere is no connection
  update_state_(this->state_);
}

// display component
void IPixelBLE::do_update_() {
  ESP_LOGW(TAG, "display update called");
  load_gif_effect();
}

void IPixelBLE::draw_absolute_pixel_internal(int x, int y, Color color) {
  // take care to ignore x,y coordinates outside the avaialble framebuffer
  if (x >= 0 && x < state_.mDisplayWidth && y >= 0 && y < state_.mDisplayHeight && state_.framebuffer_.size() > 0)
  {
    int i = (y * state_.mDisplayWidth + x) * 3; // 3 bytes per pixel

    state_.framebuffer_[i + 0] = color.red;
    state_.framebuffer_[i + 1] = color.green;
    state_.framebuffer_[i + 2] = color.blue;
  }
}

// ble client component
void IPixelBLE::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                          esp_ble_gattc_cb_param_t *param) {
  // for events refer to https://github.com/espressif/esp-idf/blob/master/components/bt/host/bluedroid/api/include/api/esp_gattc_api.h
  // for return unions refer to: https://sourcevu.sysprogs.com/espressif/esp-idf/symbols/esp_ble_gattc_cb_param_t

  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_OPEN_EVT status:%d conn_id=%d *remote_bda=0x%x mtu=%d", param->open.status, param->open.conn_id, param->open.remote_bda, param->open.mtu);
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_DISCONNECT_EVT reason:%d conn_id=%d *remote_bda=0x%x", param->disconnect.reason, param->disconnect.conn_id, param->disconnect.remote_bda);
      this->node_state = esp32_ble_tracker::ClientState::DISCONNECTING;
      this->handle_ = 0;
	    this->state_.mConnectionState = 0;
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGW(TAG, "GATTC_SEARCH_CMPL_EVT status:%d conn_id=%d source=%d", param->search_cmpl.status, param->search_cmpl.conn_id, param->search_cmpl.searched_service_source);
      // register for notify
      this->ble_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda());
      break;

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      ESP_LOGW(TAG, "GATTC_REG_FOR_NOTIFY_EVT status:%d handle=%d", param->reg_for_notify.status, param->reg_for_notify.handle);
      if (param->reg_for_notify.status == ESP_GATT_OK) {
        // now we are connected to the device and can collect some informations about the device
        this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
	      this->state_.mConnectionState = 1;
        // get writer handle
        auto *characteristic = this->parent()->get_characteristic(this->service_uuid_, this->characteristic_uuid_);
        if (characteristic != nullptr && (characteristic->properties & ESP_GATT_CHAR_PROP_BIT_WRITE) != 0) {
          this->handle_ = characteristic->handle;
        } else {
          ESP_LOGW(TAG, "Write characteristic not found.");
          break;
        }

        // get device name
        characteristic = this->parent()->get_characteristic(0x1800, ESP_GATT_UUID_GAP_DEVICE_NAME);
        if (characteristic != nullptr && (characteristic->properties & ESP_GATT_CHAR_PROP_BIT_READ) != 0) {
          // result comes via read event
          ble_read_chr(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(), characteristic->handle);
        } else {
          ESP_LOGW(TAG, "Access characteristic not found.");
        }
        
        this->is_ready_ = true;

        // initialize device: synchronize time. This commands also forces the device info notification!
        this->on_update_time_button_press();  // triggers time update
      }
      break;

    case ESP_GATTC_NOTIFY_EVT:
      ESP_LOGW(TAG, "GATTC_NOTIFY_EVT conn_id=%d handle=%d len=%d is_notify=%d", param->notify.conn_id, param->notify.handle, param->notify.value_len, param->notify.is_notify);
      if (param->notify.conn_id == this->parent()->get_conn_id()) {
        std::vector<uint8_t> data(param->notify.value, param->notify.value + param->notify.value_len);
        this->on_notification_received(data);
      }
      break;

    case ESP_GATTC_CFG_MTU_EVT: // Maximun Transfer Unit
      ESP_LOGW(TAG, "ESP_GATTC_CFG_MTU_EVT status: %d, conn_id=%d, mtu=%d ", param->cfg_mtu.status, param->cfg_mtu.conn_id, param->cfg_mtu.mtu);
      break;

    case ESP_GATTC_CONNECT_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_CONNECT_EVT conn_id=%d link_role=%d conn_handle=%d", param->connect.conn_id, param->connect.link_role, param->connect.conn_handle);
      ESP_LOGW(TAG, "conn_params: interval=%d latency=%d timeout=%d", param->connect.conn_params.interval, param->connect.conn_params.latency, param->connect.conn_params.timeout);
      break;
    
    case ESP_GATTC_SEARCH_RES_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_SEARCH_RES_EVT");
      ESP_LOGW(TAG, "conn_id=%d", param-> search_res.conn_id);
      ESP_LOGW(TAG, "start_hanle=%d", param->search_res.start_handle);
      ESP_LOGW(TAG, "end_handle=%d", param->search_res.end_handle);
      ESP_LOGW(TAG, "srvc_id: uuid={len=%d uuid=0x%x} inst_id=%d", param->search_res.srvc_id.uuid.len, param->search_res.srvc_id.uuid.uuid.uuid16, param->search_res.srvc_id.inst_id);
      ESP_LOGW(TAG, "is_primary: %s", param->search_res.is_primary ? "true": "false");
      break;

    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_DIS_SRVC_CMPL_EVT status=%d conn_id=%d", param->dis_srvc_cmpl.status, param->dis_srvc_cmpl.conn_id);
      break;

    case ESP_GATTC_READ_CHAR_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_READ_CHAR_EVT status=%d conn_id=%d handle=%d len=%d", param->read.status, param->read.conn_id, param->read.handle, param->read.value_len);
      if (param->read.status == ESP_GATT_OK) {
        // this is the only read command ever send. (access service / device name)
        std::string data(param->read.value, param->read.value + param->read.value_len);
        device_info_.set_name(data);
      }
      break;
    
    case ESP_GATTC_WRITE_CHAR_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_WRITE_CHAR_EVT status=%d conn_id=%d handle=%d offset=%d", param->write.status, param->write.conn_id, param->write.handle, param->write.offset);
      break;

    case ESP_GATTC_CLOSE_EVT:
      ESP_LOGW(TAG, "ESP_GATTC_CLOSE_EVT status=%d conn_id=%d *remote_bda=0x%x reason=%d", param->close.status, param->close.conn_id, param->close.remote_bda, param->close.reason);
      break;

    default:
      ESP_LOGW(TAG, "Unhandled GATT event: %d", event);
      break;
  }
}

bool IPixelBLE::ble_write_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle, uint8_t *data,
                                    uint16_t len) {
  esp_err_t ret = esp_ble_gattc_write_char(gattc_if, 0, handle, len, data, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "Write characteristic failed, status: %d", ret);
    return false;
  }
  return true;
}

bool IPixelBLE::ble_read_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle) {
  esp_err_t ret = esp_ble_gattc_read_char(gattc_if, 0, handle, ESP_GATT_AUTH_REQ_NONE);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "Read characteristic failed, status: %d", ret);
    return false;
  }
  return true;
}

bool IPixelBLE::ble_register_for_notify(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda) {
  auto *chr = this->parent()->get_characteristic(this->service_uuid_, this->notify_uuid_);
  if (chr != nullptr && (chr->properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY) != 0) {
    esp_err_t ret = esp_ble_gattc_register_for_notify(gattc_if, remote_bda, chr->handle);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "Register for notify failed, status: %d", ret);
      return false;
    }
  } else {
    ESP_LOGW(TAG, "Notify characteristic not found.");
    return false;
  }
  return true;
}

void IPixelBLE::on_notification_received(const std::vector<uint8_t> &data) {
  ESP_LOGW(TAG, "Notification received: %s", format_hex_pretty(data).c_str());
  // each command send to the device notifys with a 5 byte response starting with 0x05. Ignore these notifys!
  if (device_info_.get_device_info(data)) {
    // if no explicit display size set in esphome yaml file, use the display size detected
    if (state_.mDisplayWidth == 0) { 
      state_.mDisplayWidth = device_info_.width_;
    }
    if (state_.mDisplayHeight == 0) {
      state_.mDisplayHeight = device_info_.height_;
    }
    size_t new_size = state_.mDisplayWidth * state_.mDisplayHeight * 3;
    if (state_.framebuffer_.size() != new_size) { 
      // allocate framebuffer
      state_.framebuffer_.resize(new_size);
    }
  } else {
    if (data[0] == 0x05 && data[4] == 3) {
      ESP_LOGW(TAG, "sucess");
    } 
  }
  upload_queue_->publish_state(0);
  this->is_ready_ = true;
}

void IPixelBLE::write_state(light::LightState *state) {
  if (!state->get_effect_name().compare("None")) {
    state_.effect_ = None;
  }
  float fbrightness, fred,  fgreen, fblue;
  state->set_gamma_correct(0.0);  // avoid gamma correction on RGB values
  state->current_values_as_brightness(&fbrightness);
  state->current_values_as_rgb(&fred, &fgreen, &fblue);
  uint8_t r = (fred * 255); // / fbrightness;   // revert the brightness correction of color value
  uint8_t g = (fgreen * 255); // / fbrightness;
  uint8_t b = (fblue * 255); // / fbrightness;
  uint8_t brightness = fbrightness * 100;
  bool color_changed = false;
  ESP_LOGW(TAG, "brightness: %d r: %d g: %d b: %d", brightness, r, g, b);

  bool on = true;
  state->current_values_as_binary(&on);
  if (state_.mPowerState != on) {
    state_.mPowerState = on;
    if (on) {
      queuePush( iPixelCommads::setPower(true) );
      state_.effect_ = state_.mEffect;
      //state_.mEffectRestore = true;
    }
    else {
      queuePush( iPixelCommads::setPower(false) );
	  }
  }
  if (!on) return;

  if (state_.mR != r) {
    state_.mR = r;
    color_changed = true;
  }

  if (state_.mG != g) {
    state_.mG = g;
    color_changed = true;
  }

  if (state_.mB != b) {
    state_.mB = b;
    color_changed = true;
  }

  if (state_.mBrightness != brightness) {
    state_.mBrightness = brightness;
    queuePush( iPixelCommads::setBrightness( state_.mBrightness ) );
  }
  
  if ((state_.mEffect != state_.effect_ || color_changed) && !is_starting()) {
    if (play_switch_ != nullptr && play_switch_->state) {
      on_play_switch(false);
    }
    if (state_.mEffect == RandomPixels) { // restore brightness
       is_ready_ = true;
       upload_queue_->publish_state(0);
    }
    if (state_.mEffect == Alarm) { // restore brightness
       queuePush( iPixelCommads::setBrightness( state_.mBrightness ) );
    }
    state_.mEffect = state_.effect_;
    //state_.mEffectPtr = state->get_effect_by_index(state->get_current_effect_index());

    switch (state_.effect_)
    {
      case None:
        queuePush(iPixelCommads::clear());
        break;
      case Time:
        state_.mShowDate = false;
        time_date_effect();
        break;
      case TimeDate:
        state_.mShowDate = true;
        time_date_effect();
        break;
      case Message:
        text_effect();
        break;
      case LoadImage:
        load_image_effect();
        break;
      case LoadGIF:
        load_gif_effect();
        break;
      case FillColor:
        fill_color_effect();
        break;
      case FillRainbow:
        fill_rainbow_effect();
        break;
      case RandomPixels:
        random_pixel_effect();
        break;
      case Alarm:
        alarm_effect();
        break;
      default:
        break; 
    }
  }
}

void IPixelBLE::update_state_(const DeviceState &new_state) {
  // numbers
  if (annimation_mode_number_ != nullptr && annimation_mode_number_->state != new_state.mAnimationMode) {
    annimation_mode_number_->publish_state(new_state.mAnimationMode);
  }
  if (annimation_speed_number_ != nullptr && annimation_speed_number_->state != new_state.mAnimationSpeed) {
    annimation_speed_number_->publish_state(new_state.mAnimationSpeed);
  }
  if (clock_style_number_ != nullptr && clock_style_number_->state != new_state.mClockStyle) {
    clock_style_number_->publish_state(new_state.mClockStyle);
  }
  if (font_flag_number_ != nullptr && font_flag_number_->state != new_state.mFontFlag) {
    font_flag_number_->publish_state(new_state.mFontFlag);
  }
  if (lambda_slot_number_ != nullptr  && lambda_slot_number_->state != new_state.mSlotNumber) {
     lambda_slot_number_->publish_state(new_state.mSlotNumber);
  }
  if (text_mode_number_ != nullptr && text_mode_number_->state != new_state.mTextMode) {
    text_mode_number_->publish_state(new_state.mTextMode);
  }
  // sensors
  if (connect_state_ != nullptr && connect_state_->state != new_state.mConnectionState) {
    connect_state_->publish_state(new_state.mConnectionState);
  }
  if (display_height_ != nullptr && display_height_->state != new_state.mDisplayHeight) {
    display_height_->publish_state(new_state.mDisplayHeight);
  }
  if (display_width_ != nullptr && display_width_->state != new_state.mDisplayWidth) {
    display_width_->publish_state(new_state.mDisplayWidth);
  }
  if (font_height_ != nullptr && font_height_->state != new_state.mFontHeight) {
    font_height_->publish_state(new_state.mFontHeight);
  }
  if (font_width_ != nullptr && font_width_->state != new_state.mFontWidth) {
    font_width_->publish_state(new_state.mFontWidth);
  }
  if (fun_mode_ != nullptr && fun_mode_->state != new_state.mFunMode) {
    fun_mode_->publish_state(new_state.mFunMode);
  }
  if (orientation_ != nullptr && orientation_->state != new_state.mOrientation) {
    orientation_->publish_state(new_state.mOrientation);
  }
}

bool IPixelBLE::queueTick() {
    if (queue_.empty() || !is_ready_) return false;
    
    //Get command from queue
    std::vector<uint8_t> &command = queue_.front();

    //Take bytes from command
    size_t chunkSize = std::min(500, (int)command.size());

    //Write bytes from command
    bool write_ok = ble_write_chr(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(), this->handle_, command.data(), chunkSize);

    //Remove bytes from command
    command.erase(command.begin(), command.begin() + chunkSize);

    //Remove command if empty
    if (command.empty()) {
      queue_.erase(queue_.begin());
      is_ready_ = false; // wait for notification
      delay_microseconds_safe(100000L);
    }

    // disconnect if write fails
    if (!write_ok) {
      ESP_LOGE(TAG, "BLE write failed or no ACK received, disconnecting...");
      client->disconnect();
      is_ready_ = true;
      upload_queue_->publish_state(0);
    } else {
      upload_queue_->publish_state(upload_queue_->state + 1);
    }
    return write_ok;
}

void IPixelBLE::queuePush(std::vector<uint8_t> command) {
    uint32_t len = command.size();
    ESP_LOGW(TAG, "cmd_len: %d", len);
    if (len > 0) {
      queue_.push_back(command);
    }
}

void IPixelBLE::on_clock_style_number(float value) {
  state_.mClockStyle = value;
  time_date_effect();
}

void IPixelBLE::on_lambda_slot_number(float value) {
  if (state_.mSlotNumber != value) {
    state_.mSlotNumber = value;
    if (lambda_slot_number_ != nullptr) lambda_slot_number_->publish_state(value);
    load_image_effect();
  }
}

void IPixelBLE::on_annimation_mode_number(float value) {
  state_.mAnimationMode = value;
  if (!is_starting()) text_effect();
}

void IPixelBLE::on_annimation_speed_number(float value) {
  state_.mAnimationSpeed = value;
  if (!is_starting()) text_effect();
}

void IPixelBLE::on_font_flag_number(float value) {
  // font_flag 0: 8x16 1: 16x16 2: 16x32 fixed font size
  // font_flag 3: 8x16 4: 16x32  variable fonts size. Evaluates to 0x80 width height encoding. Not supported by 32x32 LED Pixel Board.

  uint8_t font_flag = value;
  switch (font_flag) {
    case 0:
    case 3:
      state_.mFontWidth = 8;
      state_.mFontHeight = 16;
      break;
    case 1:
      state_.mFontWidth = 16;
      state_.mFontHeight = 16;
      break;
    case 2:
    case 4:
      if (state_.mDisplayHeight < 32) {
        // avoid display crashes
        ESP_LOGE(TAG, "font height 32px is larger than display height!");
        return;
      }
      state_.mFontWidth = 16;
      state_.mFontHeight = 32;
      break;
    default:
      ESP_LOGE(TAG,"Unssupported font_flag %d", font_flag);
      return;
  } 

  state_.mFontFlag = font_flag;
  if (!is_starting()) text_effect();
}

void IPixelBLE::on_text_mode_number(float value) {
  state_.mTextMode = value;
  if (!is_starting()) text_effect();
}

void IPixelBLE::on_update_time_button_press() {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);
    // Set timezone to Europe Standard Time
    setenv("TZ", "CST-1", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    std::strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

    queuePush( iPixelCommads::setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec) );
    queuePush( iPixelCommads::notifyFirmwareVersions() );
}

void IPixelBLE::on_play_switch(bool state) {
  state_.mPlayState = state;
  // check if a list is avalable
  state_.mPlayState = (get_slot(false) > 0) ? true : false;

  // update switch state and send playlist command
  if (play_switch_ != nullptr && play_switch_->state != state_.mPlayState) {
      play_switch_->publish_state(state_.mPlayState);
      if (state_.mPlayState) {
        queuePush( iPixelCommads::startProgramList(state_.mProgramSlots) );
      } else {
        state_.mProgramSlots.clear();
        std::vector<uint8_t> live {0};
        queuePush( iPixelCommads::startProgramList(live) );
      }
  }
}

void IPixelBLE::text_effect() {
  if (state_.mEffect == Message || is_starting()) {
    queuePush( iPixelCommads::sendText(state_.txt_, state_.mAnimationMode, state_.mAnimationSpeed,
      Color{state_.mR, state_.mG, state_.mB}, state_.mTextMode, state_.mFontFlag, get_slot(),
      Color{state_.mRBack, state_.mGBack, state_.mBBack}) );
  }
}

void IPixelBLE::time_date_effect() {
  if (state_.mEffect == Time || state_.mEffect == TimeDate) {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    int wday = timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday;  // 1 to 7 where 7 is sunday
    queuePush( iPixelCommads::setClockMode(state_.mClockStyle, wday, timeinfo.tm_year - 100, timeinfo.tm_mon + 1, timeinfo.tm_mday, state_.mShowDate, true) );
  }
}

void IPixelBLE::load_image_effect(int8_t page) {
  if (state_.mEffect == LoadImage || is_starting()) {
    if (page >= 0 && page < 16) {
      state_.mSlotNumber = page;
      if (lambda_slot_number_ != nullptr) lambda_slot_number_->publish_state(page);
    }
    Display::do_update_(); // call display lambda writer
    queuePush( iPixelCommads::sendImage( state_.framebuffer_, get_slot() ) );
  }
}

void IPixelBLE::load_gif_effect() {
  if (state_.mEffect == LoadGIF) {
  }
}

void IPixelBLE::fill_color_effect() {
  // Fill framebuffer with color
  for (int x = 0; x < state_.mDisplayWidth; x++) {
    for (int y = 0; y < state_.mDisplayHeight; y++) {
      draw_pixel_at(x, y, Color(state_.mR, state_.mG, state_.mB));
    }
  }
  queuePush( iPixelCommads::sendImage( state_.framebuffer_, get_slot() ) );
}

void IPixelBLE::fill_rainbow_effect() {
  // Fill framebuffer with rainbow
  for (int x = 0; x < state_.mDisplayWidth; x++) {
    for (int y = 0; y < state_.mDisplayHeight; y++) {

      int i = (y * state_.mDisplayWidth + x) * 3; // 3 bytes per pixel shitet by x

      // Hue-based rainbow
      float h = (float(x) + float(y)) / (state_.mDisplayWidth);
      float r = fabs(sin(h * 3.14159f));
      float g = fabs(sin((h + 0.33f) * 3.14159f));
      float b = fabs(sin((h + 0.66f) * 3.14159f));

      draw_pixel_at(x, y, Color(r * 255, g * 255, b * 255));
    }
  }
  queuePush( iPixelCommads::sendImage( state_.framebuffer_, get_slot() ) );
}

float frand() { return (float) rand() / RAND_MAX; }

void IPixelBLE::random_pixel_effect() {
  if (state_.mEffect == RandomPixels) {
    srand(millis());
    uint8_t x = frand() * state_.mDisplayWidth;
    uint8_t y = frand() * state_.mDisplayHeight;
    uint8_t r = frand() * 255;
    uint8_t g = frand() * 255;
    uint8_t b = frand() * 255;
    queuePush( iPixelCommads::setPixel(x, y, r, g, b) );
    is_ready_ = true; // setPixel does not send a notification
  }
}

void IPixelBLE::alarm_effect() {
  static int index = 0;
  uint8_t b_val[] = { 100, 50, 25, 12, 0 };

  if (state_.mEffect == Alarm) {
    queuePush( iPixelCommads::setBrightness(b_val[index]) );
    index++;
    if (index > 4) index = 0;
  }
}

void IPixelBLE::set_color(std::string slot_csv) {
  std::stringstream ss(slot_csv);
  std::vector<uint8_t> color;
  unsigned int number;

  while(ss >> number) {
    if (number >= 0 && number <= 255)
    color.push_back(number);
  }

  if (color.size() == 3) {
    state_.mR = color[0];
    state_.mG = color[1];
    state_.mB = color[2];
    if (!is_starting()) text_effect();
  }
}

void IPixelBLE::set_background_color(std::string slot_csv) {
  std::stringstream ss(slot_csv);
  std::vector<uint8_t> color;
  unsigned int number;

  while(ss >> number) {
    if (number >= 0 && number <= 255)
    color.push_back(number);
  }

  if (color.size() == 3) {
    state_.mRBack = color[0];
    state_.mGBack = color[1];
    state_.mBBack = color[2];
    if (!is_starting()) text_effect();
  }
}

void IPixelBLE::set_program_list(std::string slot_csv) {
  std::stringstream ss(slot_csv);
  unsigned int number;
  state_.mProgramSlots.clear();

  while(ss >> number) {
    if (number > 0 && number <= 100)
    state_.mProgramSlots.push_back(number);
  }

  // update program slot sensor at least one slot is required to buid a valid program
  if (state_.mProgramSlots.size() >= 1) {
    get_slot(false);
  }
}

void IPixelBLE::del_program_list(std::string slot_csv) {
  std::stringstream ss(slot_csv);
  unsigned int number;
  std::vector<uint8_t> slot_list;

  while(ss >> number) {
    if (number > 0 && number <= 100)
    slot_list.push_back(number);
  }

  if (slot_list.size() > 0) {
    queuePush( iPixelCommads::deleteSlotList(slot_list) );
  } else {
    queuePush( iPixelCommads::clear() );
  }
}

uint8_t IPixelBLE::get_slot(bool countdown) {
  uint8_t slot = 0;       // this is the live slot
  uint8_t next_slot = 0;  // next slot when counddown is active

  if ((state_.mProgramSlots.size() > 0) && state_.mPlayState) {
    slot = state_.mProgramSlots[0];
    next_slot = slot;
    if (countdown) {
      next_slot = (state_.mProgramSlots.size() > 1) ? state_.mProgramSlots[1] : 0;
      state_.mProgramSlots.erase(state_.mProgramSlots.begin());
    }
  }

  // update next program slot sensor
  if (program_slot_ != nullptr && program_slot_->state != next_slot) {
    program_slot_->publish_state(next_slot);
  }

  return slot;
}

bool IPixelBLE::is_starting() {
  bool starting = play_switch_ != nullptr && play_switch_->state;

  if (starting){
    starting = state_.mProgramSlots.size() > 0;
  }

  return starting;
}

void IPixelBLE::downloadTick() {
#ifdef HAS_PSRAM
  if (buffer_.is_valid() && upload_queue_->state == 0) {
    uint8_t slot = get_slot();
    uint8_t index;
    const std::vector<uint8_t> data = buffer_.get_chunk(index);
    if (data.size() > 0) {
      upload_queue_->publish_state(1);
      queuePush( iPixelCommads::sendImage(data, slot, index, true, buffer_.get_size(), buffer_.get_crc()) );
    }
  }
#endif
} 

void IPixelBLE::load_image_url(std::string url) {
#ifdef HAS_PSRAM
  bool is_gif = url.rfind(".gif") != std::string::npos;
  ESP_LOGW(TAG, "url=%s is_gif=%d", url.c_str(), is_gif);
  if ( http_request_ != nullptr) {
    std::string body;
    std::list<http_request::Header> request_headers;
    std::set<std::string> collect_headers;
    
    buffer_.downloader_ = http_request_->get(url, request_headers, collect_headers);
    if (buffer_.downloader_ != nullptr) {
      ESP_LOGW(TAG, "downloader status=%d length=%d %dms", buffer_.downloader_->status_code, buffer_.downloader_->content_length, buffer_.downloader_->duration_ms);
      if (buffer_.downloader_->status_code == http_request::HTTP_STATUS_OK) {
        buffer_.set_size(buffer_.downloader_->content_length);
      }
    }
    upload_queue_->publish_state(0);
  }
#else
  ESP_LOGW(TAG, "requires build_flag: -D HAS_PSRAM");
#endif
}

}  // namespace ipixel_ble
}  // namespace esphome

#endif
