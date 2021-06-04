// Copyright 2021 Hiram Silvey

#include "profiles.h"

#include "Arduino.h"
#include <EEPROM.h>
#include "decoder.h"
#include "pins.h"
#include "profiles.pb.h"

using Profile = configurator_profiles_Profile;
using Platform = configurator_profiles_Profile_Platform;

// TODO(hiramj): Add support for termination byte to exit the loop.
void Profiles::Store() {
  if (digitalRead(kRightIndexExtra) != LOW) {
    return;
  }
  int address = 16;  // 0-15 reserved for joystick calibration values.
  while(true) {
    if (Serial.available() > 0) {
      EEPROM.update(address, (byte) Serial.read());
      address++;
    }
  }
}

int Profiles::GetPosition() {
  std::pair<int, int>[] button_to_position = {
                                              std::make_pair(kLeftRingExtra, 1),
                                              std::make_pair(kLeftMiddleExtra, 2),
                                              std::make_pair(kRightMiddleExtra, 3),
                                              std::make_pair(kRightMiddleRing, 4),
  };
  int position = 0; 
  for (const auto& [b, p] : button_to_position) {
    if (digitalRead(b) != LOW) {
      position = p;
      break;
    }
  }
  return position;
}

Profile Fetch(Platform platform) {
  int position = GetPosition();
  int address = Decoder::GetAddress(platform, position);
  return Decoder::Decode(address);
}
