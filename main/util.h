// Copyright 2021 Hiram Silvey

#ifndef UTIL_H_
#define UTIL_H_

#include <memory>

#include "teensy.h"

namespace hs {
namespace util {

// Write the 2 bytes from a short to EEPROM.
void WriteShortToEEPROM(const Teensy& teensy, int address, uint16_t val);

// Write the 4 bytes from an int to EEPROM.
void WriteIntToEEPROM(const Teensy& teensy, int address, int val);

// Read 2 bytes from EEPROM and return it as a single short.
int16_t ReadShortFromEEPROM(const Teensy& teensy, int address);

// Read 4 bytes from EEPROM and return it as a single integer.
int ReadIntFromEEPROM(const Teensy& teensy, int address);

}  // namespace util
}  // namespace hs

#endif  // UTIL_H_
