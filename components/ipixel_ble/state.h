#pragma once

#include "esphome/components/light/light_state.h"
#include <cinttypes>
#include <string>

namespace esphome {
namespace ipixel_ble {

enum effects {
    None,
    Time,
    TimeDate,
    Message,
    LoadImage,
    LoadGIF,
    FillColor,
    FillRainbow,
    RhythmAnimation,
    RhythmLevels,
    RandomPixels,
    Alarm
};

class DeviceState {
 public:
  enum effects effect_{None};
  enum effects mEffect{None};
  //light::LightEffect *mEffectPtr{nullptr};
  //bool mEffectRestore{false};

  // switches
  bool    mPowerState{true};
  bool    mPlayState{false};

  // sensors
  uint8_t mConnectionState{0};
  uint16_t mDisplayWidth{0};
  uint16_t mDisplayHeight{0};
  uint8_t mFontFlag{0};
  uint8_t mFontWidth{8};
  uint8_t mFontHeight{16};
  uint16_t mRotation{0};
  uint8_t mFunMode{0};
  uint8_t mProgramSlot{0};

  // numbers
  uint8_t mClockStyle{7};
  uint8_t mAnimationSpeed{100};
  uint8_t mSlotNumber{0};

  // color picker
  uint8_t mR{0xFF};
  uint8_t mG{0xFF};
  uint8_t mB{0xFF};
  uint8_t mBrightness{100};
  
  // backgrounf color
  uint8_t mRBack{0x00};
  uint8_t mGBack{0x00};
  uint8_t mBBack{0x00};

  // exclusive numbers (optipnal)
  uint8_t mXpos{0}; // setPixel 
  uint8_t mYpos{0}; // setPixel 

  bool    mShowDate{false};
  uint8_t mAnimationMode{1}; // sendText
  uint8_t mTextMode{0};      // sendText

  std::vector<uint8_t> mProgramSlots;

  // frame buffer for RGB display graphics
  std::vector<uint8_t> framebuffer_;

  // text input
  std::string txt_ = "";
};


}  // namespace ipixel_ble
}  // namespace esphome
