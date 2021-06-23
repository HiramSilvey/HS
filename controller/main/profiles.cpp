// Copyright 2021 Hiram Silvey

#include "profiles.h"

#include "Arduino.h"
#include <EEPROM.h>
#include "decoder.h"
#include "pins.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;
using Platform = hs_profile_Profile_Platform;

// TODO(hiramj): Add support for termination byte to exit the loop.
void Profiles::Store() {
  if (digitalRead(kRightIndexExtra) != LOW) {
    return;
  }
  int address = 16;  // 0-15 reserved for joystick calibration values.
  while(true) {
    if (Serial.available() > 0) {
      byte data = Serial.read();
      EEPROM.update(address, data);
      address++;
    }
  }
}

int GetPosition() {
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
  return position;
}

Layout Profiles::Fetch(Platform platform) {
  return Decoder::Decode(platform, GetPosition());
}
