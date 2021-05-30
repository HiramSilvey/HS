// Copyright 2021 Hiram Silvey

#include "profiles.h"

#include "Arduino.h"
#include <EEPROM.h>
#include "pins.h"

// TODO(hiramj): Add support for termination byte to exit the loop.
void StoreProfiles() {
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
