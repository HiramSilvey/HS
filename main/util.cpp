// Copyright 2021 Hiram Silvey

#include "util.h"

#include "Arduino.h"
#include <EEPROM.h>

int Util::GetIntFromEEPROM(int address) {
  byte one = EEPROM.read(address);
  byte two = EEPROM.read(address+1);
  byte three = EEPROM.read(address+2);
  byte four = EEPROM.read(address+3);
  return one << 24 | two << 16 | three << 8 | four;
}
