// Copyright 2021 Hiram Silvey

#include "pc_controller.h"

#include <memory>

#include "Arduino.h"
#include "hall_joystick.h"
#include "pins.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;
using Action = hs_profile_Profile_Layout_Action;

extern volatile uint8_t usb_configuration;

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

PCController::PCController() {
  button_id_to_pins_ = {};
  z_x_ = {};
  z_y_ = {};
  slider_left_ = {};
  slider_right_ = {};
  hat_up_ = {};
  hat_down_ = {};
  hat_left_ = {};
  hat_right_ = {};
}

void PCController::LoadProfile() {
  Layout layout = FetchProfile(hs_profile_Profile_Platform_PC);
  joystick_ = std::make_unique<HallJoystick>(0, 1023, layout.joystick_threshold);
  joystick_->Init();

  std::vector<Pins::ActionPin> action_pins = Pins::GetActionPins(layout);

  std::unordered_map<int, int> action_to_button_id = {
    {hs_profile_Profile_Layout_DigitalAction_X, 2},
    {hs_profile_Profile_Layout_DigitalAction_CIRCLE, 3},
    {hs_profile_Profile_Layout_DigitalAction_TRIANGLE, 4},
    {hs_profile_Profile_Layout_DigitalAction_SQUARE, 1},
    {hs_profile_Profile_Layout_DigitalAction_L1, 5},
    {hs_profile_Profile_Layout_DigitalAction_L2, 7},
    {hs_profile_Profile_Layout_DigitalAction_L3, 11},
    {hs_profile_Profile_Layout_DigitalAction_R1, 6},
    {hs_profile_Profile_Layout_DigitalAction_R2, 8},
    {hs_profile_Profile_Layout_DigitalAction_R3, 12},
    {hs_profile_Profile_Layout_DigitalAction_OPTIONS, 10},
    {hs_profile_Profile_Layout_DigitalAction_SHARE, 9}
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
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_LEFT_MIN:
          slider_left_.push_back({joystick_->get_min(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_LEFT_MAX:
          slider_left_.push_back({joystick_->get_max(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_RIGHT_MIN:
          slider_right_.push_back({joystick_->get_min(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_RIGHT_MAX:
          slider_right_.push_back({joystick_->get_max(), pin});
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_UP:
          hat_up_.push_back(pin);
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_DOWN:
          hat_down_.push_back(pin);
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_LEFT:
          hat_left_.push_back(pin);
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_RIGHT:
          hat_right_.push_back(pin);
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
      case hs_profile_Profile_Layout_AnalogAction_ID_SLIDER_LEFT:
        slider_left_.push_back({value, pin});
        break;
      case hs_profile_Profile_Layout_AnalogAction_ID_SLIDER_RIGHT:
        slider_right_.push_back({value, pin});
        break;
      default:
        break;
      }
    }
  }
}

bool PCController::Init() {
  if (!usb_configuration) {
    return false;
  }
  LoadProfile();
  Joystick.useManualSend(true);
  return true;
}

int PCController::GetDPadAngle() {
  int bits = 0;
  for (const int pin : hat_up_) {
    if (digitalRead(pin) == LOW) {
      bits |= 8;  // 1000
      break;
    }
  }
  for (const int pin : hat_down_) {
    if (digitalRead(pin) == LOW) {
      bits |= 4;  // 0100
      break;
    }
  }
  for (const int pin : hat_left_) {
    if (digitalRead(pin) == LOW) {
      bits |= 2;  // 0010
      break;
    }
  }
  for (const int pin : hat_right_) {
    if (digitalRead(pin) == LOW) {
      bits |= 1;  // 1000
      break;
    }
  }
  return kDPadAngle[bits];
}

void PCController::Loop() {
  HallJoystick::Coordinates coords = joystick_->GetCoordinates();
  Joystick.X(coords.x);
  Joystick.Y(joystick_->get_max()-coords.y);
  Joystick.Z(Controller::ResolveSOCD(z_y_, joystick_->get_neutral()));
  Joystick.Zrotate(Controller::ResolveSOCD(z_x_, joystick_->get_neutral()));
  Joystick.sliderLeft(Controller::ResolveSOCD(slider_left_, joystick_->get_neutral()));
  Joystick.sliderRight(Controller::ResolveSOCD(slider_right_, joystick_->get_neutral()));

  for (const auto& element : button_id_to_pins_) {
    bool active = 0;
    for (const auto& pin : element.second) {
      if (digitalRead(pin) == LOW) {
        active = 1;
        break;
      }
    }
    Joystick.button(element.first, active);
  }

  Joystick.hat(GetDPadAngle());
  Joystick.send_now();
}
