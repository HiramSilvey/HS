// Copyright 2021 Hiram Silvey

#ifndef HALL_SENSOR_H_
#define HALL_SENSOR_H_

class HallSensor {
public:
  virtual ~HallSensor() {}
  virtual void Setup() = 0;
  virtual void UpdateData() = 0;
  virtual float GetX() const = 0;
  virtual float GetY() const = 0;
  virtual float GetZ() const = 0;
};

#endif  // HALL_SENSOR_H_
