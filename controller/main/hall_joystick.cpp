// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <Wire.h>
#include <Tlv493d.h>
#include <EEPROM.h>

int HallJoystick::GetIntFromEEPROM(int address) {
  byte one = EEPROM.read(address);
  byte two = EEPROM.read(address+1);
  byte three = EEPROM.read(address+2);
  byte four = EEPROM.read(address+3);
  return one << 24 | two << 16 | three << 8 | four;
}

void HallJoystick::Init() {
  sensor_.begin();
  sensor_.setAccessMode(sensor_.LOWPOWERMODE);
  sensor_.disableTemp();
}

int HallJoystick::Normalize(int val, const Bounds& in) {
  return constrain(map(val, in.min, in.max, out_.min, out_.max),
                   out_.min, out_.max);
}

HallJoystick::Coordinates HallJoystick::GetCoordinates() {
  sensor_.updateData();
  float z = sensor_.getZ();
  int x = sensor_.getX() / z * 1000000;
  int y = sensor_.getY() / z * 1000000;
  return {Normalize(x, x_in_), Normalize(y, y_in_)};
}
