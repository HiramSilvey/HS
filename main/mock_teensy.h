// Copyright 2021 Hiram Silvey

#ifndef MOCK_TEENSY_H_
#define MOCK_TEENSY_H_

#include "mcu.h"

#include "gmock/gmock.h"

class MockTeensy : public MCU {
 public:
  MOCK_METHOD(bool, DigitalReadLow, (uint8_t pin), (override));
};

#endif  // MOCK_TEENSY_H_
