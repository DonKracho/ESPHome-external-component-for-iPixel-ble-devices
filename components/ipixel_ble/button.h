#pragma once

#include "esphome/components/button/button.h"
#include "ipixel_ble.h"

namespace esphome {
namespace display {

class DeleteSlotButton : public button::Button, public Parented<IPixelBLE> {
 public:
  DeleteSlotButton() = default;

 protected:
  void press_action() override { this->parent_->on_delete_slot_button_press(); }
};

class UpdateTimeButton : public button::Button, public Parented<IPixelBLE> {
 public:
  UpdateTimeButton() = default;

 protected:
  void press_action() override { this->parent_->on_update_time_button_press(); }
};

}  // namespace display
}  // namespace esphome
