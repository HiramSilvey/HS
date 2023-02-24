// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <memory>

#include "teensy.h"

namespace hs {

class HallJoystick {
 public:
  // Output axis bit precision (i.e. number of bits supported) + digital
  // joystick activation threshold.
  explicit HallJoystick(Teensy& teensy, int out_precision, int threshold);

  int get_min();
  int get_max();
  int get_neutral();

  struct Coordinates {
    int x;
    int y;
  };

  // Translate the raw input value to the expected output range.
  int Translate(int val);

  // Resolve coordinate value based on digital activation threshold.
  int ResolveDigitalCoord(int coord);

  // Read and return X and Y axes values.
  Coordinates GetCoordinates(Teensy& teensy);

 private:
  // The difference between the input and output bit precision (i.e. number of
  // bits supported).
  const int precision_diff_;

  // Output maximum and neutral values. Used for digital coordinate resolution.
  const int out_max_;
  const int out_neutral_;

  // Digital joystick activation thresholds (negative, positive).
  const std::pair<int, int> threshold_;
};

}  // namespace hs

#endif  // HALL_JOYSTICK_H_
