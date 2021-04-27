// Copyright 2021 Hiram Silvey

#include <memory>

#include "constants.h"
#include "usb_controller.h"
#include "hall_joystick.h"
#include "pins.h"

extern volatile uint8_t usb_configuration;

// Right joystick pins.
const int kZLeft = kIndexTop;
const int kZUp = kMiddleTop;
const int kZDown = kThumbMiddle;
const int kZRight = kRingTop;

// D-pad pins.
const int kDPadLeft = kLeftRingExtra;
const int kDPadDown = kLeftMiddleExtra;
const int kDPadUp = kRightMiddleExtra;
const int kDPadRight = kRightRingExtra;

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
const int kDPadUpAngle = 0;
const int kDPadRightAngle = 90;
const int kDPadDownAngle = 180;
const int kDPadLeftAngle = 270;

// Analog joystick value bounds.
const int kJoystickNeutral = 512;
const int kJoystickMin     = 0;
const int kJoystickMax     = 1023;

namespace {
  int ReadButton(int pin) {
    return kPressed ? digitalRead(pin) == LOW : kReleased;
  }

  int GetDPadAngle() {
    if (ReadButton(kDPadLeft) == kPressed) {
      return kDPadLeftAngle;
    }
    if (ReadButton(kDPadUp) == kPressed) {
      return kDPadUpAngle;
    }
    if (ReadButton(kDPadDown) == kPressed) {
      return kDPadDownAngle;
    }
    if (ReadButton(kDPadRight) == kPressed) {
      return kDPadRightAngle;
    }
    return -1;
  }
}  // namespace

USBController::USBController() {
  joystick_ = std::make_unique<HallJoystick>(kJoystickNeutral, kJoystickMin, kJoystickMax);
}

int USBController::ResolveSOCD(int low_direction, int high_direction) {
  if (digitalRead(low_direction) == LOW
      && digitalRead(high_direction) == HIGH) return kJoystickMin;
  if (digitalRead(high_direction) == LOW
      && digitalRead(low_direction) == HIGH) return kJoystickMax;
  return kJoystickNeutral;
}

bool USBController::Init() {
  if (!usb_configuration) {
    return false;
  }

  layout_[kSquare] = kThumbTop;
  layout_[kX] = kIndexMiddle;
  layout_[kCircle] = kMiddleMiddle;
  layout_[kTriangle] = kRingMiddle;
  layout_[kL1] = kPinkyMiddle;
  layout_[kR1] = kPinkyTop;
  layout_[kL3] = kThumbBottom;
  layout_[kL2] = kMiddleBottom;
  layout_[kR2] = kRingBottom;
  layout_[kR3] = kPinkyBottom;
  layout_[kShare] = kLeftIndexExtra;
  layout_[kOptions] = kRightIndexExtra;

  pinMode(kThumbTop, INPUT_PULLUP);
  pinMode(kThumbMiddle, INPUT_PULLUP);
  pinMode(kThumbBottom, INPUT_PULLUP);
  pinMode(kIndexTop, INPUT_PULLUP);
  pinMode(kIndexMiddle, INPUT_PULLUP);
  pinMode(kMiddleTop, INPUT_PULLUP);
  pinMode(kMiddleMiddle, INPUT_PULLUP);
  pinMode(kMiddleBottom, INPUT_PULLUP);
  pinMode(kRingTop, INPUT_PULLUP);
  pinMode(kRingMiddle, INPUT_PULLUP);
  pinMode(kRingBottom, INPUT_PULLUP);
  pinMode(kPinkyTop, INPUT_PULLUP);
  pinMode(kPinkyMiddle, INPUT_PULLUP);
  pinMode(kPinkyBottom, INPUT_PULLUP);
  pinMode(kLeftRingExtra, INPUT_PULLUP);
  pinMode(kLeftMiddleExtra, INPUT_PULLUP);
  pinMode(kLeftIndexExtra, INPUT_PULLUP);
  pinMode(kRightIndexExtra, INPUT_PULLUP);
  pinMode(kRightMiddleExtra, INPUT_PULLUP);
  pinMode(kRightRingExtra, INPUT_PULLUP);

  Joystick.useManualSend(true);
  return true;
}

void USBController::Loop() {
  Joystick.X(joystick_->GetX());
  Joystick.Y(joystick_->GetY());
  Joystick.Z(ResolveSOCD(kZDown, kZUp));
  Joystick.Zrotate(ResolveSOCD(kZLeft, kZRight));
  Joystick.sliderLeft(kJoystickNeutral);
  Joystick.sliderRight(kJoystickNeutral);

  for (const auto& element : layout_) {
    Joystick.button(element.first, ReadButton(element.second));
  }

  Joystick.hat(GetDPadAngle());
  Joystick.send_now();
}
