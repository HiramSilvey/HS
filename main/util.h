// Copyright 2021 Hiram Silvey

#ifndef UTIL_H_
#define UTIL_H_

class Util {
public:
  // Read 4 bytes from EEPROM and return it as a single integer.
  static int GetIntFromEEPROM(int address);
};

#endif  // UTIL_H_
