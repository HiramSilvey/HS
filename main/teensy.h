// Copyright 2021 Hiram Silvey

#ifndef TEENSY_H_
#define TEENSY_H_

#include "mcu.h"

#include "Arduino.h"

class Teensy : public MCU {
 public:
  int digitalRead(uint8_t pin) override {
    return digitalRead(pin);
  }
}

#endif  // TEENSY_H_
