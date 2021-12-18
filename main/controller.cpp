// Copyright 2021 Hiram Silvey

#include "controller.h"

#include "decoder.h"
#include "mcu.h"
#include "pins.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;
using Platform = hs_profile_Profile_Platform;

Layout Controller::FetchProfile(const Platform& platform, const std::unique_ptr<MCU>& mcu) {
  std::pair<int, int> button_to_position[] = {
                                              std::make_pair(kLeftRingExtra, 1),
                                              std::make_pair(kLeftMiddleExtra, 2),
                                              std::make_pair(kRightMiddleExtra, 3),
                                              std::make_pair(kRightRingExtra, 4),
  };
  int position = 0;
  for (const auto& element : button_to_position) {
    if (mcu->DigitalReadLow(element.first)) {
      position = element.second;
      break;
    }
  }
  return Decoder::Decode(platform, position);
}

int Controller::ResolveSOCD(const std::vector<AnalogButton>& buttons, int joystick_neutral, const std::unique_ptr<MCU>& mcu) {
  int min_value = joystick_neutral;
  int max_value = joystick_neutral;
  for (const auto& button : buttons) {
    if (mcu->DigitalReadLow(button.pin)) {
      if (button.value < min_value) {
        min_value = button.value;
      } else if (button.value > max_value) {
        max_value = button.value;
      }
    }
  }
  if (min_value != joystick_neutral && max_value != joystick_neutral) {
    return joystick_neutral;
  } else if (min_value != joystick_neutral) {
    return min_value;
  }
  return max_value;
}
