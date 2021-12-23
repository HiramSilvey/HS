// Copyright 2021 Hiram Silvey

#include "util.h"

#include <memory>

int Util::GetIntFromEEPROM(const std::unique_ptr<MCU>& mcu, int address) {
  byte one = mcu->EEPROMRead(address);
  byte two = mcu->EEPROMRead(address+1);
  byte three = mcu->EEPROMRead(address+2);
  byte four = mcu->EEPROMRead(address+3);
  return one << 24 | two << 16 | three << 8 | four;
}
