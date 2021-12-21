// Copyright 2021 Hiram Silve

#ifndef TLV493D_SENSOR_H_
#define TLV493D_SENSOR_H_

#include "hall_sensor.h"

#include <tlv493d.h>

class Tlv493dSensor : public HallSensor {
public:
  Tlv493dSensor() {
    sensor_.begin();
    sensor_.setAccessMode(sensor_.LOWPOWERMODE);
    sensor_.disableTemp();
  }
  inline void UpdateData() const override {
    sensor_.updateData();
  }
  inline float GetX() const override {
    return sensor_.getX();
  }
  inline float GetY() const override {
    return sensor_.getY();
  }
  inline float GetZ() const override {
    return sensor_.getZ();
  }
private:
  Tlv49d sensor_;
};

#endif  // TLV493D_SENSOR_H_
