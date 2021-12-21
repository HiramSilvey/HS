// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include "tlv493d_sensor.h"

class HallJoystick {
public:
  // Minimum and maximum values each joystick axis is expected to output +
  // digital joystick activation threshold.
  explicit HallJoystick(int min, int max, int threshold);

  int get_min();
  int get_max();
  int get_neutral();

  struct Coordinates {
    int x;
    int y;
  };

  // Read and return X and Y axes values.
  Coordinates GetCoordinates();

private:
  struct Bounds {
    int min;
    int max;
  };

  // Map the provided int value from the specified input range to the global
  // output range.
  int Normalize(int val, const Bounds& in);

  // Resolve coordinate value based on digital activation threshold.
  int ResolveDigitalCoord(int coord);

  // Read and return 4 consecutive bytes as an int from EEPROM, with the highest
  // order byte at the lowest address.
  int GetIntFromEEPROM(int address);

  // Input data bounds.
  Bounds x_in_;
  Bounds y_in_;

  // Output data bounds.
  const Bounds out_;
  const int out_neutral_;

  // Digital joystick activation thresholds (negative, positive).
  const std::pair<int, int> threshold_;

  Tlv493dSensor sensor_;
};

#endif  // HALL_JOYSTICK_H_
