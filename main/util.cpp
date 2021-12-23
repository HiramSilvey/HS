// Copyright 2021 Hiram Silvey

#include "util.h"

#include <memory>

int Util::GetIntFromEEPROM(const std::unique_ptr<MCU>& mcu, int address) {
  uint8_t one = mcu->EEPROMRead(address);
  uint8_t two = mcu->EEPROMRead(address+1);
  uint8_t three = mcu->EEPROMRead(address+2);
  uint8_t four = mcu->EEPROMRead(address+3);
  return one << 24 | two << 16 | three << 8 | four;
}
