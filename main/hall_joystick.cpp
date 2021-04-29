// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <Wire.h>
#include <Tlv493d.h>

void HallJoystick::Init() {
  sensor_.begin();
  sensor_.setAccessMode(sensor_.FASTMODE);
  sensor_.disableTemp();
  // Set I2C clock to recommended 1MHz for FASTMODE.
  Wire.setClock(1000000);
}

int HallJoystick::Normalize(float val, const Bounds& in) {
  int ival = val * 1000;
  return constrain(map(ival, in.min, in.max, out_.min, out_.max),
                   out_.min, out_.max);
}

int HallJoystick::GetX() {
  return Normalize(sensor_.getX(), x_in_);
}

int HallJoystick::GetY() {
  return Normalize(sensor_.getY(), y_in_);
}
