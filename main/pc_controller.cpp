// Copyright 2021 Hiram Silvey

#include "pc_controller.h"

#include <memory>

#include "controller.h"
#include "hall_joystick.h"
#include "pins.h"
#include "profile.pb.h"
#include "teensy.h"

namespace hs {

using Layout = hs_profile_Profile_Layout;
using Layer = hs_profile_Profile_Layer;
using Action = hs_profile_Profile_Layer_Action;

// DPad degrees with neutral SOCD.
const int kDPadAngle[16] = {
    // Bit order: Up, Down, Left, Right
    -1,   // 0000 None
    90,   // 0001 Right
    270,  // 0010 Left
    -1,   // 0011 Left + Right cancel
    180,  // 0100 Down
    135,  // 0101 Down + Right
    225,  // 0110 Down + Left
    180,  // 0111 Down; Left + Right cancel
    0,    // 1000 Up
    45,   // 1001 Up + Right
    315,  // 1010 Up + Left
    0,    // 1011 Up; Left + Right cancel
    -1,   // 1100 Up + Down cancel
    90,   // 1101 Right; Up + Down cancel
    270,  // 1110 Left; Up + Down cancel
    -1,   // 1111 Up + Down cancel; Left + Right cancel
};

PCController::PCController(std::unique_ptr<Teensy> teensy)
    : teensy_(std::move(teensy)), base_mapping_({}), mod_mapping_({}) {
  LoadProfile();
  teensy_->JoystickUseManualSend();
}

PCButtonPinMapping PCController::GetButtonPinMapping(const Layer& layer) {
  PCButtonPinMapping mapping;

  std::vector<pins::ActionPin> action_pins = pins::GetActionPins(layer);

  std::unordered_map<int, int> action_to_button_id = {
      {hs_profile_Profile_Layer_DigitalAction_X, 2},
      {hs_profile_Profile_Layer_DigitalAction_CIRCLE, 3},
      {hs_profile_Profile_Layer_DigitalAction_TRIANGLE, 4},
      {hs_profile_Profile_Layer_DigitalAction_SQUARE, 1},
      {hs_profile_Profile_Layer_DigitalAction_L1, 5},
      {hs_profile_Profile_Layer_DigitalAction_L2, 7},
      {hs_profile_Profile_Layer_DigitalAction_L3, 11},
      {hs_profile_Profile_Layer_DigitalAction_R1, 6},
      {hs_profile_Profile_Layer_DigitalAction_R2, 8},
      {hs_profile_Profile_Layer_DigitalAction_R3, 12},
      {hs_profile_Profile_Layer_DigitalAction_OPTIONS, 10},
      {hs_profile_Profile_Layer_DigitalAction_SHARE, 9}};

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
            mapping.z_y.push_back({joystick_->get_max(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_DOWN:
            mapping.z_y.push_back({joystick_->get_min(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_LEFT:
            mapping.z_x.push_back({joystick_->get_min(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_R_STICK_RIGHT:
            mapping.z_x.push_back({joystick_->get_max(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MIN:
            mapping.slider_left.push_back({joystick_->get_min(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX:
            mapping.slider_left.push_back({joystick_->get_max(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MIN:
            mapping.slider_right.push_back({joystick_->get_min(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MAX:
            mapping.slider_right.push_back({joystick_->get_max(), pin});
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_UP:
            mapping.hat_up.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN:
            mapping.hat_down.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT:
            mapping.hat_left.push_back(pin);
            break;
          case hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT:
            mapping.hat_right.push_back(pin);
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
        case hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT:
          mapping.slider_left.push_back({value, pin});
          break;
        case hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_RIGHT:
          mapping.slider_right.push_back({value, pin});
          break;
        default:
          break;
      }
    }
  }

  return mapping;
}

void PCController::LoadProfile() {
  Layout layout = FetchProfile(*teensy_, hs_profile_Profile_Platform_PC);
  joystick_ = std::make_unique<HallJoystick>(*teensy_, 0, 1023,
                                             layout.joystick_threshold);
  base_mapping_ = GetButtonPinMapping(layout.base);
  if (layout.has_mod) {
    mod_mapping_ = GetButtonPinMapping(layout.mod);
  }
}

int PCController::GetDPadAngle(const PCButtonPinMapping& mapping) {
  int bits = 0;
  for (const int pin : mapping.hat_up) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 8;  // 1000
      break;
    }
  }
  for (const int pin : mapping.hat_down) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 4;  // 0100
      break;
    }
  }
  for (const int pin : mapping.hat_left) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 2;  // 0010
      break;
    }
  }
  for (const int pin : mapping.hat_right) {
    if (teensy_->DigitalReadLow(pin)) {
      bits |= 1;  // 0001
      break;
    }
  }
  return kDPadAngle[bits];
}

void PCController::UpdateButtons(const PCButtonPinMapping& mapping) {
  teensy_->SetJoystickZ(
      ResolveSOCD(*teensy_, mapping.z_y, joystick_->get_neutral()));
  teensy_->SetJoystickZRotate(
      ResolveSOCD(*teensy_, mapping.z_x, joystick_->get_neutral()));
  teensy_->SetJoystickSliderLeft(
      ResolveSOCD(*teensy_, mapping.slider_left, joystick_->get_neutral()));
  teensy_->SetJoystickSliderRight(
      ResolveSOCD(*teensy_, mapping.slider_right, joystick_->get_neutral()));

  for (const auto& element : mapping.button_id_to_pins) {
    bool active = false;
    for (const auto& pin : element.second) {
      if (teensy_->DigitalReadLow(pin)) {
        active = true;
        break;
      }
    }
    teensy_->SetJoystickButton(element.first, active);
  }

  teensy_->SetJoystickHat(GetDPadAngle(mapping));
}

void PCController::Loop() {
  HallJoystick::Coordinates coords = joystick_->GetCoordinates(*teensy_);
  teensy_->SetJoystickX(coords.x);
  teensy_->SetJoystickY(joystick_->get_max() - coords.y);

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

  teensy_->JoystickSendNow();
}

}  // namespace hs
