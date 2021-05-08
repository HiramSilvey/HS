// Copyright 2021 Hiram Silvey

#include "configurator.h"

void Configure() {
  bool was_released = false;
  while(true) {
    bool is_pressed = digitalRead(kConfigureButton) == LOW;
    if (was_released && is_pressed) {
      return;
    } else if (!was_released && !is_pressed) {
      was_released = true;
    }
    if (Serial.available() > 0) {
      // TODO(hiram): Decode and store profile data.
      int data = Serial.read();
      Serial.println(data, DEC);
    }
  }
}
