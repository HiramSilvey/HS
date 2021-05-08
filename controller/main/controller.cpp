#include "controller.h"

#include "Arduino.h"
#include "pins.h"
#include "profiles.h"

void Controller::InitPins() {
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
}

int Controller::ResolveSOCD(int low_direction, int high_direction, int joystick_min,
                            int joystick_max, int joystick_neutral) {
  if (digitalRead(low_direction) == LOW
      && digitalRead(high_direction) == HIGH) return joystick_min;
  if (digitalRead(high_direction) == LOW
      && digitalRead(low_direction) == HIGH) return joystick_max;
  return joystick_neutral;
}
