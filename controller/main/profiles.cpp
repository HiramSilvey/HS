// Copyright 2021 Hiram Silvey

#include "profiles.h"

#include "Arduino.h"
#include "pins.h"

void StoreProfiles() {
  delay(100);  // Ensure button states are settled before relying on them.
  if (digitalRead(kRightIndexExtra) != LOW) {
    return;
  }
  while(true) {
    if (Serial.available() > 0) {
      int data = Serial.read();
      Serial.println(data, DEC);
    }
  }
}
