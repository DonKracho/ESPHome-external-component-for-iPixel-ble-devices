#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/log.h"
#include "ipixel_ble.h"

namespace esphome {
namespace display {

class PlaySwitch : public switch_::Switch, public Parented<IPixelBLE> {
 public:
  PlaySwitch() = default;

 protected:
  void write_state(bool state) override { this->parent_->on_play_switch(state); }
};

}  // namespace display
}  // namespace esphome
