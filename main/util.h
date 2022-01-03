// Copyright 2021 Hiram Silvey

#ifndef UTIL_H_
#define UTIL_H_

#include <memory>

#include "teensy.h"

namespace hs {
namespace util {

// Read 4 bytes from EEPROM and return it as a single integer.
int GetIntFromEEPROM(const Teensy& teensy, int address);

}  // namespace util
}  // namespace hs

#endif  // UTIL_H_
