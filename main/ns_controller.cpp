// Copyright 2021 Hiram Silvey

#include "ns_controller.h"

#include <memory>

#include "Arduino.h"
#include "hall_joystick.h"
#include "pins.h"
#include "profile.pb.h"
#include "usb_nsgamepad.h"

using Layout = hs_profile_Profile_Layout;
using Layer = hs_profile_Profile_Layer;
using Action = hs_profile_Profile_Layer_Action;

extern volatile uint8_t usb_configuration;

// DPad direction with neutral SOCD.
const int kDPadDirection[16] = {
  // Bit order: Up, Down, Left, Right
  NSGAMEPAD_DPAD_CENTERED,    // 0000 None
  NSGAMEPAD_DPAD_RIGHT,       // 0001
  NSGAMEPAD_DPAD_LEFT,        // 0010
  NSGAMEPAD_DPAD_CENTERED,    // 0011 Left + Right cancel
  NSGAMEPAD_DPAD_DOWN,        // 0100
  NSGAMEPAD_DPAD_DOWN_RIGHT,  // 0101
  NSGAMEPAD_DPAD_DOWN_LEFT,   // 0110
  NSGAMEPAD_DPAD_DOWN,        // 0111 Left + Right cancel
  NSGAMEPAD_DPAD_UP,          // 1000
  NSGAMEPAD_DPAD_UP_RIGHT,    // 1001
  NSGAMEPAD_DPAD_UP_LEFT,     // 1010
  NSGAMEPAD_DPAD_UP,          // 1011 Left + Right cancel
  NSGAMEPAD_DPAD_CENTERED,    // 1100 Up + Down cancel
  NSGAMEPAD_DPAD_RIGHT,       // 1101 Up + Down cancel
  NSGAMEPAD_DPAD_LEFT,        // 1110 Up + Down cancel
  NSGAMEPAD_DPAD_CENTERED,    // 1111 Up + Down cancel; Left + Right cancel
};

NSController::NSController() {
  base_mapping_ = {};
  mod_mapping_ = {};
}

NSController::NSButtonPinMapping NSController::GetButtonPinMapping(const Layer& layer) {
  NSButtonPinMapping mapping;

  std::vector<Pins::ActionPin> action_pins = Pins::GetActionPins(layer);

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
    if (action.which_action_type == hs_profile_Profile_Layer_Action_digital_tag) {
      auto digital = action.action_type.digital;
      if (action_to_button_id.find(digital) != action_to_button_id.end()) {
        int button_id = action_to_button_id[digital];
        if (mapping.button_id_to_pins.find(button_id) != mapping.button_id_to_pins.end()) {
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
  Layout layout = FetchProfile(hs_profile_Profile_Platform_SWITCH);
  joystick_ = std::make_unique<HallJoystick>(0, 255, layout.joystick_threshold);
  joystick_->Init();

  base_mapping_ = GetButtonPinMapping(layout.base);
  if (layout.has_mod) {
    mod_mapping_ = GetButtonPinMapping(layout.mod);
  }
}

bool NSController::Init() {
  if (!usb_configuration) {
    return false;
  }
  LoadProfile();
  return true;
}

int NSController::GetDPadDirection(const NSButtonPinMapping& mapping) {
  int bits = 0;
  for (const int pin : mapping.dpad_up) {
    if (digitalRead(pin) == LOW) {
      bits |= 8;  // 1000
      break;
    }
  }
  for (const int pin : mapping.dpad_down) {
    if (digitalRead(pin) == LOW) {
      bits |= 4;  // 0100
      break;
    }
  }
  for (const int pin : mapping.dpad_left) {
    if (digitalRead(pin) == LOW) {
      bits |= 2;  // 0010
      break;
    }
  }
  for (const int pin : mapping.dpad_right) {
    if (digitalRead(pin) == LOW) {
      bits |= 1;  // 0001
      break;
    }
  }
  return kDPadDirection[bits];
}

void NSController::UpdateButtons(const NSButtonPinMapping& mapping) {
  NSGamepad.rightYAxis(joystick_->get_max()-
                       Controller::ResolveSOCD(mapping.z_y, joystick_->get_neutral()));
  NSGamepad.rightXAxis(Controller::ResolveSOCD(mapping.z_x, joystick_->get_neutral()));

  for (const auto& element : mapping.button_id_to_pins) {
    for (const auto& pin : element.second) {
      if (digitalRead(pin) == LOW) {
        NSGamepad.press(element.first);
        break;
      }
    }
  }

  NSGamepad.dPad(GetDPadDirection(mapping));
}

void NSController::Loop() {
  NSGamepad.releaseAll();

  HallJoystick::Coordinates coords = joystick_->GetCoordinates();
  NSGamepad.leftYAxis(joystick_->get_max()-coords.y);
  NSGamepad.leftXAxis(coords.x);

  bool mod_active = false;
  for (const auto& pin : base_mapping_.mod) {
    if (digitalRead(pin) == LOW) {
      mod_active = true;
      break;
    }
  }

  if (mod_active) {
    UpdateButtons(mod_mapping_);
  } else {
    UpdateButtons(base_mapping_);
  }

  NSGamepad.loop();
}
