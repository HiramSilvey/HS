// Copyright 2021 Hiram Silvey

#include "util.h"

#include <memory>

#include "teensy.h"

namespace hs {
namespace util {

void WriteShortToEEPROM(const Teensy& teensy, int address, uint16_t val) {
  teensy.EEPROMUpdate(address, val >> 8 & 0xFF);
  teensy.EEPROMUpdate(address + 1, val & 0xFF);
}

void WriteIntToEEPROM(const Teensy& teensy, int address, int val) {
  teensy.EEPROMUpdate(address, val >> 24 & 0xFF);
  teensy.EEPROMUpdate(address + 1, val >> 16 & 0xFF);
  teensy.EEPROMUpdate(address + 2, val >> 8 & 0xFF);
  teensy.EEPROMUpdate(address + 3, val & 0xFF);
}

int16_t ReadShortFromEEPROM(const Teensy& teensy, int address) {
  uint8_t one = teensy.EEPROMRead(address);
  uint8_t two = teensy.EEPROMRead(address + 1);
  return one << 8 | two;
}

int ReadIntFromEEPROM(const Teensy& teensy, int address) {
  uint8_t one = teensy.EEPROMRead(address);
  uint8_t two = teensy.EEPROMRead(address + 1);
  uint8_t three = teensy.EEPROMRead(address + 2);
  uint8_t four = teensy.EEPROMRead(address + 3);
  return one << 24 | two << 16 | three << 8 | four;
}

}  // namespace util
}  // namespace hs
