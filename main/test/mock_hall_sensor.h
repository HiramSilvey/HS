// Copyright 2021 Hiram Silve

#ifndef MOCK_HALL_SENSOR_H_
#define MOCK_HALL_SENSOR_H_

#include "hall_sensor.h"

#include "gmock/gmock.h"

class MockHallSensor : public HallSensor {
public:
  MOCK_METHOD(void, UpdateData, (), (override));
  MOCK_METHOD(float, GetX, (), (const, override));
  MOCK_METHOD(float, GetY, (), (const, override));
  MOCK_METHOD(float, GetZ, (), (const, override));
};

#endif  // MOCK_HALL_SENSOR_H_
