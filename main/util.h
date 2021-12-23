// Copyright 2021 Hiram Silvey

#ifndef UTIL_H_
#define UTIL_H_

#include <memory>

#include "teensy.h"

class Util {
 public:
  // Read 4 bytes from EEPROM and return it as a single integer.
  static int GetIntFromEEPROM(const std::unique_ptr<Teensy>& teensy,
                              int address);
};

#endif  // UTIL_H_
