// Copyright 2021 Hiram Silvey

#ifndef MOCK_NSPAD_H_
#define MOCK_NSPAD_H_

#include "nspad.h"

#include "gmock/gmock.h"

class MockNSPad : public NSPad {
 public:
  MOCK_METHOD(int, DPadCentered, (), (const override));
  MOCK_METHOD(int, DPadUp, (), (const override));
  MOCK_METHOD(int, DPadDown, (), (const override));
  MOCK_METHOD(int, DPadLeft, (), (const override));
  MOCK_METHOD(int, DPadRight, (), (const override));
  MOCK_METHOD(int, DPadUpLeft, (), (const override));
  MOCK_METHOD(int, DPadUpRight, (), (const override));
  MOCK_METHOD(int, DPadDownLeft, (), (const override));
  MOCK_METHOD(int, DPadDownRight, (), (const override));
  MOCK_METHOD(void, SetLeftXAxis, (uint8_t val), (const override));
  MOCK_METHOD(void, SetLeftYAxis, (uint8_t val), (const override));
  MOCK_METHOD(void, SetRightXAxis, (uint8_t val), (const override));
  MOCK_METHOD(void, SetRightYAxis, (uint8_t val), (const override));
  MOCK_METHOD(void, SetDPad, (int8_t direction), (const override));
  MOCK_METHOD(void, Press, (uint8_t button_id), (const override));
  MOCK_METHOD(void, ReleaseAll, (), (const override));
  MOCK_METHOD(void, Loop, (), (const override));
};

#endif  // MOCK_NSPAD_H_
