// Copyright 2021 Hiram Silvey

#ifndef TEENSY_H_
#define TEENSY_H_

#include "mcu.h"

#include <EEPROM.h>

#include "Arduino.h"

class Teensy : public MCU {
public:
  inline int Constrain(int amount, int low, int high) const override {
    return constrain(amount, low, high);
  }
  inline bool DigitalReadLow(uint8_t pin) const override {
    return digitalRead(pin) == LOW;
  }
  inline uint8_t EEPROMRead(int addr) const override {
    return EEPROM.read(addr);
  }
};

#endif  // TEENSY_H_
