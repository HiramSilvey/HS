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
  x_in_.min = GetIntFromEEPROM(0);
  x_in_.max = GetIntFromEEPROM(4);
  y_in_.min = GetIntFromEEPROM(8);
  y_in_.max = GetIntFromEEPROM(12);

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

HallJoystick::Coordinates HallJoystick::GetCoordinates() {
  sensor_.updateData();
  return {Normalize(sensor_.getX(), x_in_), Normalize(sensor_.getY(), y_in_)};
}
