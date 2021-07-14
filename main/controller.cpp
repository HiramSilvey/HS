// Copyright 2021 Hiram Silvey

#include "controller.h"

#include "Arduino.h"
#include "decoder.h"
#include "pins.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;
using Platform = hs_profile_Profile_Platform;

Layout Controller::FetchProfile(Platform platform) {
  std::pair<int, int> button_to_position[] = {
                                              std::make_pair(kLeftRingExtra, 1),
                                              std::make_pair(kLeftMiddleExtra, 2),
                                              std::make_pair(kRightMiddleExtra, 3),
                                              std::make_pair(kRightMiddleExtra, 4),
  };
  int position = 0;
  for (const auto& element : button_to_position) {
    if (digitalRead(element.first) == LOW) {
      position = element.second;
      break;
    }
  }
  return Decoder::Decode(platform, position);
}
