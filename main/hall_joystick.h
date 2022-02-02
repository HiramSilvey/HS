// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <memory>

#include "teensy.h"

namespace hs {

class HallJoystick {
 public:
  // Minimum and maximum values each joystick axis is expected to output +
  // digital joystick activation threshold.
  explicit HallJoystick(const Teensy& teensy, int min, int max, int threshold);

  int get_min();
  int get_max();
  int get_neutral();

  struct Coordinates {
    int x;
    int y;
  };

  struct Bounds {
    int min;
    int max;
  };

  // Map the provided int value from the specified input range to the global
  // output range.
  int Normalize(const Teensy& teensy, double val, const Bounds& in);

  // Resolve coordinate value based on digital activation threshold.
  int ResolveDigitalCoord(int coord);

  // Read and return X and Y axes values.
  Coordinates GetCoordinates(Teensy& teensy);

 private:
  // Input data bounds and rotation angle.
  Bounds x_in_;
  Bounds y_in_;
  double angle_;

  // Output data bounds.
  const Bounds out_;
  const int out_neutral_;

  // Digital joystick activation thresholds (negative, positive).
  const std::pair<int, int> threshold_;

  // A buffer holding the current joystick coordinates that can be used when the
  // sensor data isn't ready yet.
  Coordinates curr_coords_;

  // Time since the last sensor data fetch.
  unsigned long last_fetch_micros_;
};

}  // namespace hs

#endif  // HALL_JOYSTICK_H_
