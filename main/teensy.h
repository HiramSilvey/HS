// Copyright 2021 Hiram Silvey

#ifndef TEENSY_H_
#define TEENSY_H_

#include "mcu.h"

#include "Arduino.h"

class Teensy : public MCU {
public:
  inline bool DigitalReadLow(uint8_t pin) const override {
    return digitalRead(pin) == LOW;
  }
};

#endif  // TEENSY_H_
