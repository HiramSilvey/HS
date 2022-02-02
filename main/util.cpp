// Copyright 2021 Hiram Silvey

#include "util.h"

#include <memory>

#include "teensy.h"

namespace hs {
namespace util {

int16_t GetShortFromEEPROM(const Teensy& teensy, int address) {
  uint8_t one = teensy.EEPROMRead(address);
  uint8_t two = teensy.EEPROMRead(address + 1);
  return one << 8 | two;
}

int GetIntFromEEPROM(const Teensy& teensy, int address) {
  uint8_t one = teensy.EEPROMRead(address);
  uint8_t two = teensy.EEPROMRead(address + 1);
  uint8_t three = teensy.EEPROMRead(address + 2);
  uint8_t four = teensy.EEPROMRead(address + 3);
  return one << 24 | two << 16 | three << 8 | four;
}

}  // namespace util
}  // namespace hs
