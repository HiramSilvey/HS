// Copyright 2021 Hiram Silvey

#include "main/constants.h"
#include "main/usb_controller.h"
#include "main/joystick.h"
#include "main/pins.h"

// Button IDs.
const int kSquare = 1;
const int kX = 2;
const int kCircle = 3;
const int kTriangle = 4;
const int kL1 = 5;
const int kR1 = 6;
const int kL2 = 7;
const int kR2 = 8;
const int kShare = 9;
const int kOptions = 10;
const int kL3 = 11;
const int kR3 = 12;

// D-pad degrees.
const int kDPadRight = 0;
const int kDPadUp = 90;
const int kDPadLeft = 180;
const int kDPadDown = 270;

// Analog joystick value bounds.
const int kJoystickNeutral = 512;
const int kJoystickMin     = 0;
const int kJoystickMax     = 1023;

int USBController::ResolveSOCD(int low_direction, int high_direction) {
  if (digitalRead(low_direction) == LOW
      && digitalRead(high_direction) == HIGH) return kJoystickMin;
  if (digitalRead(high_direction) == LOW
      && digitalRead(low_direction) == HIGH) return kJoystickMax;
  return kJoystickNeutral;
}

bool USBController::Init() {
  // TODO(hiram): implement
  return false;
}

void USBController::Loop() {
  // TODO(hiram): implement
  return;
}
