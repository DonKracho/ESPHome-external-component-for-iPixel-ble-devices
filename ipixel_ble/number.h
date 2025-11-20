#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/component.h"

#include "ipixel_ble.h"

namespace esphome {
namespace ipixel_ble {

class ClockStyleNumber : public number::Number, public Parented<IPixelBLE> {
 public:
  ClockStyleNumber() = default;

 protected:
  void control(float value) override { this->parent_->on_clock_style_number(value); }
};

class SlotNumberNumber : public number::Number, public Parented<IPixelBLE> {
 public:
  SlotNumberNumber() = default;

 protected:
  void control(float value) override { this->parent_->on_slot_number_number(value); }
};

class AnnimationModeNumber : public number::Number, public Parented<IPixelBLE> {
 public:
  AnnimationModeNumber() = default;

 protected:
  void control(float value) override { this->parent_->on_annimation_mode_number(value); }
};

class AnnimationSpeedNumber : public number::Number, public Parented<IPixelBLE> {
 public:
  AnnimationSpeedNumber() = default;

 protected:
  void control(float value) override { this->parent_->on_annimation_speed_number(value); }
};

class FontFlagNumber : public number::Number, public Parented<IPixelBLE> {
 public:
  FontFlagNumber() = default;

 protected:
  void control(float value) override { this->parent_->on_font_flag_number(value); }
};

class TextModeNumber : public number::Number, public Parented<IPixelBLE> {
 public:
  TextModeNumber() = default;

 protected:
  void control(float value) override { this->parent_->on_text_mode_number(value); }
};

}  // namespace ipixel_ble
}  // namespace esphome
