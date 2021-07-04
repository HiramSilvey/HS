// Copyright 2021 Hiram Silvey

#include "ns_controller.h"

#include <memory>

#include "Arduino.h"
#include "hall_joystick.h"
#include "pins.h"
#include "profiles.h"
#include "profile.pb.h"
#include "usb_nsgamepad.h"

using Layout = hs_profile_Profile_Layout;
using Action = hs_profile_Profile_Layout_Action;

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
  joystick_ = std::make_unique<HallJoystick>(0, 255);
  joystick_->Init();
  button_id_to_pins_ = {};
  z_x_ = {};
  z_y_ = {};
  dpad_up_ = {};
  dpad_down_ = {};
  dpad_left_ = {};
  dpad_right_ = {};
}

void NSController::LoadProfile() {
  Layout layout = Profiles::Fetch(hs_profile_Profile_Platform_SWITCH);
  std::vector<Pins::ActionPin> action_pins = Pins::GetActionPins(layout);

  std::unordered_map<int, int> action_to_button_id = {
    {hs_profile_Profile_Layout_DigitalAction_X, 1},
    {hs_profile_Profile_Layout_DigitalAction_CIRCLE, 2},
    {hs_profile_Profile_Layout_DigitalAction_TRIANGLE, 3},
    {hs_profile_Profile_Layout_DigitalAction_SQUARE, 0},
    {hs_profile_Profile_Layout_DigitalAction_L1, 4},
    {hs_profile_Profile_Layout_DigitalAction_L2, 6},
    {hs_profile_Profile_Layout_DigitalAction_L3, 10},
    {hs_profile_Profile_Layout_DigitalAction_R1, 5},
    {hs_profile_Profile_Layout_DigitalAction_R2, 7},
    {hs_profile_Profile_Layout_DigitalAction_R3, 11},
    {hs_profile_Profile_Layout_DigitalAction_OPTIONS, 9},
    {hs_profile_Profile_Layout_DigitalAction_SHARE, 8},
    {hs_profile_Profile_Layout_DigitalAction_HOME, 12},
    {hs_profile_Profile_Layout_DigitalAction_CAPTURE, 13},
  };

  for (const auto& action_pin : action_pins) {
    auto action = action_pin.action;
    int pin = action_pin.pin;
    if (action.which_action_type == hs_profile_Profile_Layout_Action_digital_tag) {
      auto digital = action.action_type.digital;
      if (action_to_button_id.find(digital) != action_to_button_id.end()) {
        int button_id = action_to_button_id[digital];
        if (button_id_to_pins_.find(button_id) != button_id_to_pins_.end()) {
          button_id_to_pins_[button_id].push_back(pin);
        } else {
          button_id_to_pins_[button_id] = {pin};
        }
      } else {
        switch (digital) {
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_UP:
          z_y_.push_back({joystick_->get_max(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_DOWN:
          z_y_.push_back({joystick_->get_min(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_LEFT:
          z_x_.push_back({joystick_->get_min(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_RIGHT:
          z_x_.push_back({joystick_->get_max(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_UP:
          dpad_up_.push_back(pin);
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_DOWN:
          dpad_down_.push_back(pin);
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_LEFT:
          dpad_left_.push_back(pin);
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_RIGHT:
          dpad_right_.push_back(pin);
          break;
        default:
          break;
        }
      }
    } else {
      auto analog = action.action_type.analog;
      int value = analog.value;
      switch (analog.id) {
      case hs_profile_Profile_Layout_AnalogAction_ID_R_STICK_X:
        z_x_.push_back({value, pin});
        break;
      case hs_profile_Profile_Layout_AnalogAction_ID_R_STICK_Y:
        z_y_.push_back({value, pin});
        break;
      default:
        break;
      }
    }
  }
}

bool NSController::Init() {
  if (!usb_configuration) {
    return false;
  }

  Pins::Init();
  Profiles::Store();  // Handle configuration mode start, no-op otherwise.
  LoadProfile();
  return true;
}

int NSController::ResolveSOCD(std::vector<AnalogButton> buttons) {
  int neutral = joystick_->get_neutral();
  int min_value = neutral;
  int max_value = neutral;
  for (const auto& button : buttons) {
    if (digitalRead(button.pin) == LOW) {
      if (button.value < min_value) {
        min_value = button.value;
      } else if (button.value > max_value) {
        max_value = button.value;
      }
    }
  }
  if (min_value != neutral && max_value != neutral) {
    return neutral;
  } else if (min_value != neutral) {
    return min_value;
  }
  return max_value;
}

int NSController::GetDPadDirection() {
  int bits = 0;
  for (const int pin : dpad_up_) {
    if (digitalRead(pin) == LOW) {
      bits |= 8;  // 1000
      break;
    }
  }
  for (const int pin : dpad_down_) {
    if (digitalRead(pin) == LOW) {
      bits |= 4;  // 0100
      break;
    }
  }
  for (const int pin : dpad_left_) {
    if (digitalRead(pin) == LOW) {
      bits |= 2;  // 0010
      break;
    }
  }
  for (const int pin : dpad_right_) {
    if (digitalRead(pin) == LOW) {
      bits |= 1;  // 1000
      break;
    }
  }
  return kDPadDirection[bits];
}

void NSController::Loop() {
  NSGamepad.releaseAll();
  HallJoystick::Coordinates coords = joystick_->GetCoordinates();

  for (const auto& element : button_id_to_pins_) {
    for (const auto& pin : element.second) {
      if (digitalRead(pin) == LOW) {
        NSGamepad.press(element.first);
        break;
      }
    }
  }

  NSGamepad.dPad(GetDPadDirection());

  NSGamepad.leftYAxis(joystick_->get_max()-coords.y);
  NSGamepad.leftXAxis(coords.x);
  NSGamepad.rightYAxis(joystick_->get_max()-ResolveSOCD(z_y_));
  NSGamepad.rightXAxis(ResolveSOCD(z_x_));

  NSGamepad.loop();
}
