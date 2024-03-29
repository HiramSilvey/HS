// Copyright 2024 Hiram Silvey

#include "controller.h"

#include "decoder.h"
#include "pins.h"
#include "profile.pb.h"
#include "teensy.h"

namespace hs {

using Layout = ::hs_profile_Profile_Layout;
using Platform = ::hs_profile_Profile_Platform;

Layout FetchProfile(const Teensy& teensy, const Platform& platform) {
  std::pair<int, int> button_to_position[] = {
      std::make_pair(pins::kIndexTop, 1),
      std::make_pair(pins::kMiddleTop, 2),
      std::make_pair(pins::kRingTop, 3),
      std::make_pair(pins::kPinkyTop, 4),
      std::make_pair(pins::kIndexMiddle, 5),
      std::make_pair(pins::kMiddleMiddle, 6),
      std::make_pair(pins::kRingMiddle, 7),
      std::make_pair(pins::kPinkyMiddle, 8),
      std::make_pair(pins::kThumbTop, 9),
      std::make_pair(pins::kMiddleBottom, 10),
      std::make_pair(pins::kRingBottom, 11),
      std::make_pair(pins::kPinkyBottom, 12)};
  int position = 0;
  for (const auto& element : button_to_position) {
    if (teensy.DigitalReadLow(element.first)) {
      position = element.second;
      break;
    }
  }
  return decoder::Decode(teensy, platform, position);
}

int ResolveSOCD(const Teensy& teensy, const std::vector<AnalogButton>& buttons,
                int joystick_neutral) {
  int min_value = joystick_neutral;
  int max_value = joystick_neutral;
  for (const auto& button : buttons) {
    if (teensy.DigitalReadLow(button.pin)) {
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

}  // namespace hs
