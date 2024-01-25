// Copyright 2024 Hiram Silvey

#ifndef UTIL_H_
#define UTIL_H_

#include <memory>

#include "teensy.h"

namespace hs {
namespace util {

// Read 2 bytes from EEPROM and return it as a single short.
int16_t GetShortFromEEPROM(const Teensy& teensy, int address);

// Read 4 bytes from EEPROM and return it as a single integer.
int GetIntFromEEPROM(const Teensy& teensy, int address);

}  // namespace util
}  // namespace hs

#endif  // UTIL_H_
