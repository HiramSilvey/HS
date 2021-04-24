// Copyright 2021 Hiram Silvey

#include <GamecubeAPI.h>
#include "main/constants.h"
#include "main/ngc_controller.h"
#include "main/joystick.h"
#include "main/pins.h"

// Default output data values.
const int kJoystickNeutral   = 128;
const int kJoystickMin       = 0;
const int kJoystickMax       = 255;
const int kTriggerLightPress = 74;

// C-stick pins.
const int kCstickLeft  = kIndexTop;
const int kCstickDown  = kThumbMiddle;
const int kCstickUp    = kMiddleTop;
const int kCstickRight = kRingTop;

// Trigger pins.
const int kL      = kIndexMiddle;
const int kRLight = kPinkyMiddle;

// Button pins.
const int kA     = kThumbTop;
const int kB     = kRingMiddle;
const int kX     = kPinkyTop;
const int kY     = kMiddleMiddle;
const int kZ     = kThumbBottom;
const int kStart = kRightIndexExtra;

// D-pad pins.
const int kDLeft  = kLeftRingExtra;
const int kDDown  = kLeftMiddleExtra;
const int kDUp    = kRightMiddleExtra;
const int kDRight = kRightRingExtra;

int NGCController::ResolveSOCD(int low_direction, int high_direction) {
  if (digitalRead(low_direction) == LOW
      && digitalRead(high_direction) == HIGH) return kJoystickMin;
  if (digitalRead(high_direction) == LOW
      && digitalRead(low_direction) == HIGH) return kJoystickMax;
  return kJoystickNeutral;
}

bool NGCController::Init() {
  controller_ = CGamecubeController(kGameCubeController);
  if (!controller_.read()) {
    return false;
  }

  output_ = defaultGamecubeData;
  console_ = CGamecubeConsole(kGameCubeConsole);
  joystick_ = Joystick({.neutral = kJoystickNeutral,
                        .min = kJoystickMin,
                        .max = kJoystickMax});

  pinMode(kCLeft, INPUT_PULLUP);
  pinMode(kCDown, INPUT_PULLUP);
  pinMode(kCUp, INPUT_PULLUP);
  pinMode(kCRight, INPUT_PULLUP);
  pinMode(kL, INPUT_PULLUP);
  pinMode(kRLight, INPUT_PULLUP);
  pinMode(kA, INPUT_PULLUP);
  pinMode(kB, INPUT_PULLUP);
  pinMode(kX, INPUT_PULLUP);
  pinMode(kY, INPUT_PULLUP);
  pinMode(kZ, INPUT_PULLUP);
  pinMode(kStart, INPUT_PULLUP);
  pinMode(kDLeft, INPUT_PULLUP);
  pinMode(kDDown, INPUT_PULLUP);
  pinMode(kDUp, INPUT_PULLUP);
  pinMode(kDRight, INPUT_PULLUP);

  joystick_.Setup();
  return true;
}

void NGCController::Loop() {
  int c_x = HandleSOCD(kCstickLeft, kCstickRight);
  int c_y = HandleSOCD(kCstickDown, kCstickUp);

  output_.report.xAxis = joystick_.GetX();
  output_.report.yAxis = joystick_.GetY();
  output_.report.cxAxis = c_x;
  output_.report.cyAxis = c_y;
  output_.report.l = kPressed ? digitalRead(kL) == LOW : kReleased;
  output_.report.right =
    kTriggerLightPress ? digitalRead(kRLight) == LOW : kReleased;
  output_.report.a = kPressed ? digitalRead(kA) == LOW : kReleased;
  output_.report.b = kPressed ? digitalRead(kB) == LOW : kReleased;
  output_.report.x = kPressed ? digitalRead(kX) == LOW : kReleased;
  output_.report.y = kPressed ? digitalRead(kY) == LOW : kReleased;
  output_.report.z = kPressed ? digitalRead(kZ) == LOW : kReleased;
  output_.report.start = kPressed ? digitalRead(kStart) == LOW : kReleased;
  output_.report.dup = kPressed ? digitalRead(kDUp) == LOW : kReleased;
  output_.report.ddown = kPressed ? digitalRead(kDDown) == LOW : kReleased;
  output_.report.dleft = kPressed ? digitalRead(kDLeft) == LOW : kReleased;
  output_.report.dright = kPressed ? digitalRead(kDRight) == LOW : kReleased;

  console_.write(output_);
}
