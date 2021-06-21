// Copyright 2021 Hiram Silvey

#include "usb_controller.h"

#include <memory>

#include "Arduino.h"
#include "hall_joystick.h"
#include "pins.h"
#include "profiles.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;
using Action = hs_profile_Profile_Layout_Action;

extern volatile uint8_t usb_configuration;

// D-pad degrees.
const int kDPadUpAngle = 0;
const int kDPadRightAngle = 90;
const int kDPadDownAngle = 180;
const int kDPadLeftAngle = 270;

USBController::USBController() {
  joystick_ = std::make_unique<HallJoystick>(0, 1023);
  joystick_->Init();
  button_id_to_pins_ = {};
  z_up_ = {0, -1};
  z_down_ = {0, -1};
  z_left_ = {0, -1};
  z_right_ = {0, -1};
  slider_left_low_ = {0, -1};
  slider_left_high_ = {0, -1};
  slider_right_low_ = {0, -1};
  slider_right_high_ = {0, -1};
  hat_up_ = -1;
  hat_down_ = -1;
  hat_left_ = -1;
  hat_right_ = -1;
}

void USBController::LoadProfile() {
  Layout layout = Profiles::Fetch(hs_profile_Profile_Platform_PC);
  // PrintLayout(layout);
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
          z_up_.value = joystick_->get_max();
          z_up_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_DOWN:
          z_down_.value = joystick_->get_min();
          z_down_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_LEFT:
          z_left_.value = joystick_->get_min();
          z_left_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_R_STICK_RIGHT:
          z_right_.value = joystick_->get_max();
          z_right_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_LEFT_MIN:
          slider_left_low_.value = joystick_->get_min();
          slider_left_low_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_LEFT_MAX:
          slider_left_high_.value = joystick_->get_max();
          slider_left_high_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_RIGHT_MIN:
          slider_right_low_.value = joystick_->get_min();
          slider_right_low_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_SLIDER_RIGHT_MAX:
          slider_right_high_.value = joystick_->get_max();
          slider_right_high_.pin = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_UP:
          hat_up_ = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_DOWN:
          hat_down_ = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_LEFT:
          hat_left_ = pin;
          break;
        case hs_profile_Profile_Layout_DigitalAction_D_PAD_RIGHT:
          hat_right_ = pin;
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
        {
          AnalogButton* z_x = value < joystick_->get_neutral() ?
            &z_left_ : &z_right_;
          z_x->value = value;
          z_x->pin = pin;
          break;
        }
      case hs_profile_Profile_Layout_AnalogAction_ID_R_STICK_Y:
        {
          AnalogButton* z_y = value < joystick_->get_neutral() ?
            &z_down_ : &z_up_;
          z_y->value = value;
          z_y->pin = pin;
          break;
        }
      case hs_profile_Profile_Layout_AnalogAction_ID_SLIDER_LEFT:
        {
          AnalogButton* slider_left = value < joystick_->get_neutral() ?
            &slider_left_low_ : &slider_left_high_;
          slider_left->value = value;
          slider_left->pin = pin;
          break;
        }
      case hs_profile_Profile_Layout_AnalogAction_ID_SLIDER_RIGHT:
        {
          AnalogButton* slider_right = value < joystick_->get_neutral() ?
            &slider_right_low_ : &slider_right_high_;
          slider_right->value = value;
          slider_right->pin = pin;
          break;
        }
      default:
        break;
      }
    }
  }
}

bool USBController::Init() {
  if (!usb_configuration) {
    return false;
  }

  Pins::Init();
  Profiles::Store();  // Handle configuration mode start, no-op otherwise.
  LoadProfile();
  Joystick.useManualSend(true);
  return true;
}

int USBController::GetDPadAngle() {
  if (hat_left_ && digitalRead(hat_left_) == LOW) {
    return kDPadLeftAngle;
  }
  if (hat_up_ && digitalRead(hat_up_) == LOW) {
    return kDPadUpAngle;
  }
  if (hat_down_ && digitalRead(hat_down_) == LOW) {
    return kDPadDownAngle;
  }
  if (hat_right_ && digitalRead(hat_right_) == LOW) {
    return kDPadRightAngle;
  }
  return -1;
}

void USBController::Loop() {
  HallJoystick::Coordinates coords = joystick_->GetCoordinates();
  Joystick.X(coords.x);
  Joystick.Y(joystick_->get_max()-coords.y);
  Joystick.Z(ResolveSOCD(z_down_.pin, z_up_.pin, z_down_.value, z_up_.value,
                         joystick_->get_neutral()));
  Joystick.Zrotate(ResolveSOCD(z_left_.pin, z_right_.pin, z_left_.value,
                               z_right_.value, joystick_->get_neutral()));
  Joystick.sliderLeft(ResolveSOCD(slider_left_low_.pin, slider_left_high_.pin,
                                  slider_left_low_.value,
                                  slider_left_high_.value,
                                  joystick_->get_neutral()));
  Joystick.sliderRight(ResolveSOCD(slider_right_low_.pin,
                                   slider_right_high_.pin,
                                   slider_right_low_.value,
                                   slider_right_high_.value,
                                   joystick_->get_neutral()));

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
