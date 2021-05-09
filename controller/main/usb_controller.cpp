// Copyright 2021 Hiram Silvey

#include "usb_controller.h"

#include <memory>

#include "Arduino.h"
#include "hall_joystick.h"
#include "pins.h"
#include "profiles.h"

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

// Super Smash Bros. Melee specific value.
const int kMeleeLightShield = 661;

namespace {
  int GetDPadAngle() {
    if (digitalRead(kDPadLeft) == LOW) {
      return kDPadLeftAngle;
    }
    if (digitalRead(kDPadUp) == LOW) {
      return kDPadUpAngle;
    }
    if (digitalRead(kDPadDown) == LOW) {
      return kDPadDownAngle;
    }
    if (digitalRead(kDPadRight) == LOW) {
      return kDPadRightAngle;
    }
    return -1;
  }
}  // namespace

USBController::USBController() {
  joystick_ = std::make_unique<HallJoystick>(0, 1023);
  joystick_->Init();
}

void USBController::LoadProfile() {
  layout_[kSquare] = kRingMiddle;
  layout_[kX] = kThumbTop;
  layout_[kCircle] = kPinkyTop;
  layout_[kTriangle] = kMiddleMiddle;
  layout_[kL1] = kMiddleBottom;
  layout_[kR1] = kThumbBottom;
  layout_[kL2] = kIndexMiddle;
  // layout_[kR2] = kPinkyMiddle;
  layout_[kShare] = kLeftIndexExtra;
  layout_[kOptions] = kRightIndexExtra;
  layout_[kL3] = kRingBottom;
  layout_[kR3] = kPinkyBottom;
}

bool USBController::Init() {
  if (!usb_configuration) {
    return false;
  }

  InitPins();
  StoreProfiles();  // Handle configuration mode start, no-op otherwise.
  LoadProfile();
  Joystick.useManualSend(true);
  return true;
}

void USBController::Loop() {
  HallJoystick::Coordinates coords = joystick_->GetCoordinates();
  Joystick.X(coords.x);
  Joystick.Y(joystick_->get_max()-coords.y);
  Joystick.Z(ResolveSOCD(kZDown, kZUp, joystick_->get_min(),
                         joystick_->get_max(), joystick_->get_neutral()));
  Joystick.Zrotate(ResolveSOCD(kZLeft, kZRight, joystick_->get_min(),
                               joystick_->get_max(), joystick_->get_neutral()));
  Joystick.sliderRight(joystick_->get_neutral());

  int slider_strength = digitalRead(kPinkyMiddle) == LOW ? kMeleeLightShield : joystick_->get_neutral();
  Joystick.sliderLeft(slider_strength);

  for (const auto& element : layout_) {
    Joystick.button(element.first, digitalRead(element.second) == LOW);
  }

  Joystick.hat(GetDPadAngle());
  Joystick.send_now();
}
