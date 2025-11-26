#include "ipixel_ble.h"
#include "iPixelCommands.h"
#include <cstdlib>  // rand
#include <cmath>    // sin
#include <algorithm>
#include <ctime>

#undef ESPHOME_LOG_LEVEL
#define ESPHOME_LOG_LEVEL ESPHOME_LOG_LEVEL_DEBUG
#include "esphome/core/log.h"

#ifdef USE_ESP32

namespace esphome {
namespace display {

const char *TAG = "ipixel_ble";

// component
void IPixelBLE::setup() {
  // allocate framebuffer
  state_.framebuffer_.resize(state_.mDisplayWidth * state_.mDisplayWidth * 3);
}

void IPixelBLE::loop() {
  if (this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED) {
    uint32_t tick = millis();
    if (this->last_request_ + (1000 *3600) < tick) {  // once per hour
      this->last_request_ = tick;
      on_update_time_button_press();  // trigger time update
    }
    
    if (this->last_animation_ + 300 < tick) {
      this->last_animation_ = tick;
      if (state_.mPowerState) {
        random_pixel_effect(); // update the animated effect whie it is selected
        alarm_effect();
      } 
    }
    queueTick();
  }
  // update sensors and numbers even tere is no connection
  update_state_(this->state_);
}

// display component
void IPixelBLE::update() {
  //ESP_LOGD(TAG, "display update called");
}

void IPixelBLE::draw_pixel_at(int x, int y, Color color) {
  // take care to ignore x,y coordinates outside the avaialble framebuffer
  if (x >= 0 && x < state_.mDisplayWidth && y >= 0 && y < state_.mDisplayHeight)
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

  /* refer to https://github.com/espressif/esp-idf/blob/master/components/bt/host/bluedroid/api/include/api/esp_gattc_api.h
  connection log:
  [12:59:58][D][esp32_ble_client:154]: [0] [0F:D6:54:F9:D2:C7] ESP_GATTC_CONNECT_EVT
  [12:59:58][D][ipixel_ble:085]: Unhandled GATT event: 40
  [12:59:58][D][esp32_ble_client:154]: [0] [0F:D6:54:F9:D2:C7] ESP_GATTC_OPEN_EVT
  [12:59:58][D][ipixel_ble:044]: GATT client opened.
  [12:59:58][D][esp32_ble_tracker:115]: connecting: 0, discovered: 0, searching: 0, disconnecting: 0
  [12:59:58][D][esp32_ble_tracker:282]: Starting scan...
  [13:00:00][D][esp32_ble_client:352]: [0] [0F:D6:54:F9:D2:C7] Event 46
  [13:00:00][D][ipixel_ble:085]: Unhandled GATT event: 46
  [13:00:01][D][ipixel_ble:085]: Unhandled GATT event: 7
  [13:00:01][D][ipixel_ble:085]: Unhandled GATT event: 7
  [13:00:01][D][ipixel_ble:085]: Unhandled GATT event: 7
  [13:00:01][D][esp32_ble_client:154]: [0] [0F:D6:54:F9:D2:C7] ESP_GATTC_SEARCH_CMPL_EVT
  [13:00:01][I][esp32_ble_client:273]: [0] [0F:D6:54:F9:D2:C7] Connected
  [13:00:01][D][esp32_ble_client:233]: [0] [0F:D6:54:F9:D2:C7] cfg_mtu status 0, mtu 517
  [13:00:01][D][ipixel_ble:085]: Unhandled GATT event: 18
  [13:00:01][D][esp32_ble_client:154]: [0] [0F:D6:54:F9:D2:C7] ESP_GATTC_REG_FOR_NOTIFY_EVT
  [13:00:01][W][esp32_ble_client:320]: [0] [0F:D6:54:F9:D2:C7] esp_ble_gattc_get_descr_by_char_handle error, status=10
  [13:00:01][D][ble_client:057]: All clients established, services released
  */

  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      ESP_LOGD(TAG, "GATT client opened.");
	    this->state_.mConnectionState = 1;
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGD(TAG, "GATT client disconnected.");
      this->node_state = esp32_ble_tracker::ClientState::DISCONNECTING;
      this->handle_ = 0;
	    this->state_.mConnectionState = 0;
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      if (param->search_cmpl.status != ESP_GATT_OK) {
        ESP_LOGD(TAG, "Service search failed, status: %d", param->search_cmpl.status);
        break;
      }
      this->ble_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda());
      break;

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      if (param->reg_for_notify.status != ESP_GATT_OK) {
        ESP_LOGD(TAG, "Register for notify failed, status: %d", param->reg_for_notify.status);
        break;
      }
      this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
      break;

    case ESP_GATTC_NOTIFY_EVT:
      if (param->notify.conn_id != this->parent()->get_conn_id())
        break;
      if (param->notify.handle == this->handle_) {
        std::vector<uint8_t> data(param->notify.value, param->notify.value + param->notify.value_len);
        this->on_notification_received(data);
      }
      break;

    case ESP_GATTC_CFG_MTU_EVT:
      if (param->cfg_mtu.status != ESP_GATT_OK) {
        ESP_LOGD(TAG, "Config mtu failed, status: %d", param->cfg_mtu.status);
        break;
      }
      else {
        ESP_LOGD(TAG, "GATTC_CFG_MTU_EVT: %d", param->cfg_mtu.mtu);
      }
      break;

    case ESP_GATTC_READ_CHAR_EVT:
    case ESP_GATTC_WRITE_CHAR_EVT:
      break;

    case ESP_GATTC_SEARCH_RES_EVT:    // 7  This event is triggered each time a service result is obtained using `esp_ble_gattc_search_service`.
    //case ESP_GATTC_CFG_MTU_EVT:       // 18 This event is triggered upon the completion of the MTU configuration with `esp_ble_gattc_send_mtu_req`. sends: cfg_mtu status 0, mtu 517
    case ESP_GATTC_CONNECT_EVT:       // 40 This event is triggered when the physical connection is set up.
    case ESP_GATTC_DIS_SRVC_CMPL_EVT: // 46 This event is triggered when the GATT service discovery is completed.
    default:
      ESP_LOGD(TAG, "Unhandled GATT event: %d", event);
      break;
  }
}

bool IPixelBLE::ble_write_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle, uint8_t *data,
                                    uint16_t len) {
  esp_err_t ret = esp_ble_gattc_write_char(gattc_if, 0, handle, len, data, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Write characteristic failed, status: %d", ret);
    return false;
  }
  return true;
}

bool IPixelBLE::ble_read_chr(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda, uint16_t handle) {
  esp_err_t ret = esp_ble_gattc_read_char(gattc_if, 0, handle, ESP_GATT_AUTH_REQ_NONE);
  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Read characteristic failed, status: %d", ret);
    return false;
  }
  return true;
}

bool IPixelBLE::ble_register_for_notify(esp_gatt_if_t gattc_if, esp_bd_addr_t remote_bda) {
  auto *chr = this->parent()->get_characteristic(this->service_uuid_, this->characteristic_uuid_);
  if (!chr) {
    ESP_LOGD(TAG, "Characteristic not found.");
    return false;
  }
  this->handle_ = chr->handle;
  esp_err_t ret = esp_ble_gattc_register_for_notify(gattc_if, remote_bda, this->handle_);
  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Register for notify failed, status: %d", ret);
    return false;
  }
  return true;
}

void IPixelBLE::on_notification_received(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Notification received: %s", format_hex_pretty(data).c_str());
}

void IPixelBLE::control(const std::string &value) {
  ESP_LOGD(TAG, "Text has been set: %s", value.c_str());
  if (value.find("PNG:") == 0) {
    state_.png_ = value.substr(4);
    load_png_effect();
  }
  else if (value.find("GIF:") == 0) {
    state_.gif_ = value.substr(4);
    load_gif_effect();
  } 
  else {
    state_.txt_ = value;
    text_effect();
  }
}

void IPixelBLE::write_state(light::LightState *state) {
  if (!state->get_effect_name().compare("None")) state->set_flash_transition_length(0);
  enum effects effect = static_cast<enum effects>(state->get_flash_transition_length());
  float fbrightness, fred,  fgreen, fblue;
  state->set_gamma_correct(0.0);  // avoid gamma correction on RGB values
  state->current_values_as_brightness(&fbrightness);
  state->current_values_as_rgb(&fred, &fgreen, &fblue);
  uint8_t r = (fred * 255); // / fbrightness;   // revert the brightness correction of color value
  uint8_t g = (fgreen * 255); // / fbrightness;
  uint8_t b = (fblue * 255); // / fbrightness;
  uint8_t brightness = fbrightness * 100;
  bool color_changed = false;
  ESP_LOGD(TAG, "brightness: %d r: %d g: %d b: %d", brightness, r, g, b);

  bool on = true;
  state->current_values_as_binary(&on);
  if (state_.mPowerState != on) {
    state_.mPowerState = on;
    if (on) {
      queuePush( iPixelCommads::ledOn() );
      // restore last known effect
      effect = state_.mEffect;
      //auto *effect = state->get_effect_by_index(state_.mEffectIndex);
      //if (effect != nullptr) effect->apply(); 
    }
    else {
      queuePush( iPixelCommads::ledOff() );
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
  
  if (state_.mEffect != effect || color_changed) {
    if (state_.mEffect == Alarm) { // restore brightness
       queuePush( iPixelCommads::setBrightness( state_.mBrightness ) );
    }
    state_.mEffect = effect;
    //state_.mEffectIndex = state->get_current_effect_index();
    switch (effect)
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
      case LoadPNG:
        load_png_effect();
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
  // switch
  if (play_switch_ != nullptr && play_switch_->state != new_state.mPlayState) {
      play_switch_->publish_state(new_state.mPlayState);
  }
  // numbers
  if (annimation_mode_number_ != nullptr && annimation_mode_number_->state != new_state.mAnimationMode) {
    annimation_mode_number_->publish_state(new_state.mAnimationMode);
  }
  if (annimation_speed_number_ != nullptr && annimation_speed_number_->state != new_state.mAnimationSpeed) {
    annimation_speed_number_->publish_state(new_state.mAnimationSpeed);
  }
  if (font_flag_number_ != nullptr && font_flag_number_->state != new_state.mFontFlag) {
    font_flag_number_->publish_state(new_state.mFontFlag);
  }
  if (clock_style_number_ != nullptr && clock_style_number_->state != new_state.mClockStyle) {
    clock_style_number_->publish_state(new_state.mClockStyle);
  }
  if (slot_number_number_ != nullptr && slot_number_number_->state != new_state.mSlotNumber) {
    slot_number_number_->publish_state(new_state.mSlotNumber);
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

void IPixelBLE::queueTick() {
    if (queue.empty()) return;
    //Get command from queue
    std::vector<uint8_t> &command = queue.front();

    //Take bytes from command
    size_t chunkSize = std::min(500, (int)command.size());

    //Write bytes from command
    ble_write_chr(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(), this->handle_, command.data(), chunkSize);

    //Remove bytes from command
    command.erase(command.begin(), command.begin() + chunkSize);

    //Remove command if empty
    if (command.empty()) queue.erase(queue.begin());

    //Do not overload BLE
    delay(100);
}

void IPixelBLE::queuePush(std::vector<uint8_t> command) {
    uint32_t len = command.size();
    ESP_LOGD(TAG, "cmd_len: %d", len);
    if (len > 0) {
      queue.push_back(command);
    }
}

void IPixelBLE::on_clock_style_number(float value) {
  state_.mClockStyle = value;
  time_date_effect();
}

void IPixelBLE::on_slot_number_number(float value) {
  state_.mSlotNumber = value;
}

void IPixelBLE::on_annimation_mode_number(float value) {
  state_.mAnimationMode = value;
  text_effect();
}

void IPixelBLE::on_annimation_speed_number(float value) {
  state_.mAnimationSpeed = value;
  text_effect();
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
  text_effect();
}

void IPixelBLE::on_text_mode_number(float value) {
  state_.mTextMode = value;
  text_effect();
}

void IPixelBLE::on_delete_slot_button_press() {
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
}

void IPixelBLE::on_play_switch(bool state) {
  // reserved for program play
  state_.mPlayState = state;
}

void IPixelBLE::text_effect() {
  if (state_.mEffect == Message) {
    queuePush( iPixelCommads::sendText(state_.txt_, state_.mAnimationMode, state_.mSlotNumber, state_.mAnimationSpeed, state_.mR, state_.mG, state_.mB, state_.mTextMode, state_.mFontFlag) );
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

void IPixelBLE::load_png_effect() {
  if (state_.mEffect == LoadPNG) {
    queuePush( iPixelCommads::sendPNG(Helpers::hexStringToVector(state_.png_)) );
  }
}

void IPixelBLE::load_gif_effect() {
  if (state_.mEffect == LoadGIF) {
    // to test some graphic functions dwap a smiley
    filled_circle(15, 15, 14, Color(255, 255, 0));
    line(8, 20, 24, 20, Color(255, 0, 0));
    line(10, 21, 22, 21, Color(255, 0, 0));
    line(12, 22, 20, 22, Color(255, 0, 0));
    rectangle( 0, 0, 32, 32, Color(0, 0, 255));
    filled_circle(9, 9, 5, Color(255, 255, 255));
    filled_circle(22, 9, 5, Color(255, 255, 255));
    filled_circle(10, 10, 2, Color(0, 0, 0));
    filled_circle(23, 10, 2, Color(0, 0, 0));

    queuePush( iPixelCommads::sendPNG( state_.framebuffer_ ) );
  }
}

void IPixelBLE::fill_color_effect() {
  const uint8_t width = state_.mDisplayWidth;
  const uint8_t height = state_.mDisplayHeight;

  // Fill framebuffer with color
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      draw_pixel_at(x, y, Color(state_.mR, state_.mG, state_.mB));
    }
  }
  queuePush( iPixelCommads::sendPNG( state_.framebuffer_ ) );
}

void IPixelBLE::fill_rainbow_effect() {
  const uint8_t width = state_.mDisplayWidth;
  const uint8_t height = state_.mDisplayHeight;

  // Fill framebuffer with rainbow
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {

      int i = (y * width + x) * 3; // 3 bytes per pixel shitet by x

      // Hue-based rainbow
      float h = (float(x) + float(y)) / (width);
      float r = fabs(sin(h * 3.14159f));
      float g = fabs(sin((h + 0.33f) * 3.14159f));
      float b = fabs(sin((h + 0.66f) * 3.14159f));

      draw_pixel_at(x, y, Color(r * 255, g * 255, b * 255));
    }
  }
  queuePush( iPixelCommads::sendPNG( state_.framebuffer_ ) );
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
    //ESP_LOGD(TAG, "RAND_MAX: %d x: %d y: %d r: %d g: %d b: %d", RAND_MAX, x, y, r, g, b);
    queuePush( iPixelCommads::setPixel(x, y, r, g, b) );
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

}  // namespace display
}  // namespace esphome

#endif
