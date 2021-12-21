// Copyright 2021 Hiram Silvey

#ifndef HALL_SENSOR_H_
#define HALL_SENSOR_H_

class HallSensor {
public:
  virtual ~HallSensor() {}
  virtual void UpdateData() = 0;
  virtual float GetX() = 0;
  virtual float GetY() = 0;
  virtual float GetZ() = 0;
};

#endif  // HALL_SENSOR_H_
