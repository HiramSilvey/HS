// Copyright 2021 Hiram Silvey

#ifndef MOCK_MCU_H_
#define MOCK_MCU_H_

#include "mcu.h"

#include "gmock/gmock.h"

class MockMCU : public MCU {
 public:
  MOCK_METHOD(bool, DigitalReadLow, (uint8_t pin), (override));
};

#endif  // MOCK_MCU_H_
