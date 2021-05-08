// Copyright 2021 Hiram Silvey

#include "profiles.h"

#include "Arduino.h"
#include "pins.h"

void StoreProfiles() {
  if (digitalRead(kRightIndexExtra) != LOW) {
    return;
  }
  while(true) {
    if (Serial.available() > 0) {
      // TODO(hiram): Decode and store profile data.
      int data = Serial.read();
      Serial.println(data, DEC);
    }
  }
}
