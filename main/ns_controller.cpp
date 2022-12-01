// Copyright 2021 Hiram Silvey

#include "ns_controller.h"

#include <memory>

#include "hall_joystick.h"
#include "nspad.h"
#include "pins.h"
#include "profile.pb.h"
#include "teensy.h"

namespace hs {

using Layout = hs_profile_Profile_Layout;
using Layer = hs_profile_Profile_Layer;
using Action = hs_profile_Profile_Layer_Action;

NSController::NSController(std::unique_ptr<Teensy> teensy,
                           std::unique_ptr<NSPad> nspad)
    : teensy_(std::move(teensy)),
      nspad_(std::move(nspad)),
      base_mapping_({}),
      mod_mapping_({}) {
  // DPad direction with neutral SOCD. Bit order: Up, Down, Left, Right
  dpad_direction_[0] = nspad_->DPadCentered();   // 0000 None
  dpad_direction_[1] = nspad_->DPadRight();      // 0001
  dpad_direction_[2] = nspad_->DPadLeft();       // 0010
  dpad_direction_[3] = nspad_->DPadCentered();   // 0011 Left + Right cancel
  dpad_direction_[4] = nspad_->DPadDown();       // 0100
  dpad_direction_[5] = nspad_->DPadDownRight();  // 0101
  dpad_direction_[6] = nspad_->DPadDownLeft();   // 0110
  dpad_direction_[7] = nspad_->DPadDown();       // 0111 Left + Right cancel
  dpad_direction_[8] = nspad_->DPadUp();         // 1000
  dpad_direction_[9] = nspad_->DPadUpRight();    // 1001
  dpad_direction_[10] = nspad_->DPadUpLeft();    // 1010
  dpad_direction_[11] = nspad_->DPadUp();        // 1011 Left + Right cancel
  dpad_direction_[12] = nspad_->DPadCentered();  // 1100 Up + Down cancel
  dpad_direction_[13] = nspad_->DPadRight();     // 1101 Up + Down cancel
  dpad_direction_[14] = nspad_->DPadLeft();      // 1110 Up + Down cancel
  dpad_direction_[15] =
      nspad_->DPadCentered();  // 1111 Up + Down cancel; Left + Right cancel

  LoadProfile();
}

NSButtonPinMapping NSController::GetButtonPinMapping(const Layer& layer) {
  NSButtonPinMapping mapping;

  std::vector<pins::ActionPin> action_pins = pins::GetActionPins(layer);

  std::unordered_map<int, int> action_to_button_id = {
      {hs_profile_Profile_Layer_DigitalAction_X, 1},
      {hs_profile_Profile_Layer_DigitalAction_CIRCLE, 2},
      {hs_profile_Profile_Layer_DigitalAction_TRIANGLE, 3},
      {hs_profile_Profile_Layer_DigitalAction_SQUARE, 0},
      {hs_profile_Profile_Layer_DigitalAction_L1, 4},
      {hs_profile_Profile_Layer_DigitalAction_L2, 6},
      {hs_profile_Profile_Layer_DigitalAction_L3, 10},
      {hs_profile_Profile_Layer_DigitalAction_R1, 5},
      {hs_profile_Profile_Layer_DigitalAction_R2, 7},
      {hs_profile_Profile_Layer_DigitalAction_R3, 11},
      {hs_profile_Profile_Layer_DigitalAction_OPTIONS, 9},
      {hs_profile_Profile_Layer_DigitalAction_SHARE, 8},
      {hs_profile_Profile_Layer_DigitalAction_HOME, 12},
      {hs_profile_Profile_Layer_DigitalAction_CAPTURE, 13},
  };

  for (const auto& action_pin : action_pins) {
    auto action = action_pin.action;
    int pin = action_pin.pin;
    if (action.which_action_type ==
        hs_profile_Profile_Layer_Action_digital_tag) {
      auto digital = action.action_type.digital;
      if (action_to_button_id.find(digital) != action_to_button_id.end()) {
        int button_id = action_to_button_id[digital];
        if (mapping.button_id_to_pins.find(button_id) !=
            mapping.button_id_to_pins.end()) {
          mapping.button_id_to_pins[button_id].push_back(pin);
        } else {
          mapping.button_id_to_pins[button_id] = {pin};
        }
      } else {
        switch (digital) {
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_UP:
            mapping.z_y.push_back({joystick_->out_max(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_DOWN:
            mapping.z_y.push_back({joystick_->out_min(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_LEFT:
            mapping.z_x.push_back({joystick_->out_min(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_RIGHT:
            mapping.z_x.push_back({joystick_->out_max(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_UP:
            mapping.dpad_up.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN:
            mapping.dpad_down.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT:
            mapping.dpad_left.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT:
            mapping.dpad_right.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_MOD:
            mapping.mod.push_back(pin);
            break;
          default:
            break;
        }
      }
    } else {
      auto analog = action.action_type.analog;
      int value = analog.value;
      switch (analog.id) {
        case hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X:
          mapping.z_x.push_back({value, pin});
          break;
        case hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y:
          mapping.z_y.push_back({value, pin});
          break;
        default:
          break;
      }
    }
  }

  return mapping;
}

void NSController::LoadProfile() {
  Layout layout = FetchProfile(*teensy_, hs_profile_Profile_Platform_SWITCH);
  joystick_ = std::make_unique<HallJoystick>(*teensy_, 0, 255,
                                             layout.joystick_threshold);
  base_mapping_ = GetButtonPinMapping(layout.base);
  if (layout.has_mod) {
    mod_mapping_ = GetButtonPinMapping(layout.mod);
  }
}

int NSController::GetDPadDirection(const NSButtonPinMapping& mapping) {
  int bits = 0;
  for (const int pin : mapping.dpad_up) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 8;  // 1000
      break;
    }
  }
  for (const int pin : mapping.dpad_down) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 4;  // 0100
      break;
    }
  }
  for (const int pin : mapping.dpad_left) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 2;  // 0010
      break;
    }
  }
  for (const int pin : mapping.dpad_right) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 1;  // 0001
      break;
    }
  }
  return dpad_direction_[bits];
}

void NSController::UpdateButtons(const NSButtonPinMapping& mapping) {
  nspad_->SetRightYAxis(
      joystick_->out_max() -
      ResolveSOCD(*teensy_, mapping.z_y, joystick_->out_neutral()));
  nspad_->SetRightXAxis(
      ResolveSOCD(*teensy_, mapping.z_x, joystick_->out_neutral()));

  for (const auto& element : mapping.button_id_to_pins) {
    for (const auto& pin : element.second) {
      if (teensy_->DigitalReadLow(pin)) {
        nspad_->Press(element.first);
        break;
      }
    }
  }

  nspad_->SetDPad(GetDPadDirection(mapping));
}

void NSController::Loop() {
  nspad_->ReleaseAll();

  HallJoystick::Coordinates coords = joystick_->GetCoordinates(*teensy_);
  nspad_->SetLeftYAxis(joystick_->out_max() - coords.y);
  nspad_->SetLeftXAxis(coords.x);

  bool mod_active = false;
  for (const auto& pin : base_mapping_.mod) {
    if (teensy_->DigitalReadLow(pin)) {
      mod_active = true;
      break;
    }
  }

  if (mod_active) {
    UpdateButtons(mod_mapping_);
  } else {
    UpdateButtons(base_mapping_);
  }

  nspad_->Loop();
}

}  // namespace hs
