// Copyright 2021 Hiram Silvey

#ifndef MOCK_MCU_H_
#define MOCK_MCU_H_

#include "mcu.h"

#include "gmock/gmock.h"

class MockMCU : public MCU {
 public:
  MOCK_METHOD(int, Constrain, (int amount, int low, int high),
              (const override));
  MOCK_METHOD(bool, DigitalReadLow, (uint8_t pin), (const override));
  MOCK_METHOD(uint8_t, EEPROMRead, (int addr), (const override));
};

#endif  // MOCK_MCU_H_
