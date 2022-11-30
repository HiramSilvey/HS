// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <memory>

#include "math.h"
#include "teensy.h"

namespace hs {

class HallJoystick {
 public:
  // Minimum and maximum values each joystick axis is expected to output +
  // digital joystick activation threshold.
  explicit HallJoystick(const Teensy& teensy, int min, int max, int threshold);

  struct Coordinates {
    int x;
    int y;
  };

  struct Bounds {
    int min;
    int max;
  };

  inline double GetAngleFromTicks(uint16_t ticks) {
    return (M_PI * ticks) / 2000.0;
  }

  inline void set_x_in(int neutral_x, int range) {
    x_in_ = {.min = neutral_x - range, .max = neutral_x + range};
  }
  inline void set_y_in(int neutral_y, int range) {
    y_in_ = {.min = neutral_y - range, .max = neutral_y + range};
  }
  inline void set_xy_angle(uint16_t xy_angle_ticks) {
    xy_angle_ = GetAngleFromTicks(xy_angle_ticks);
  }
  inline void set_xz_angle(uint16_t xz_angle_ticks) {
    xz_angle_ = GetAngleFromTicks(xz_angle_ticks);
  }
  inline void set_yz_angle(uint16_t yz_angle_ticks) {
    yz_angle_ = GetAngleFromTicks(yz_angle_ticks);
  }
  inline void set_min(int min) { out_.min = min; }
  inline void set_max(int max) { out_.max = max; }
  inline void set_neutral(int neutral) { out_neutral_ = neutral; }
  inline void set_threshold(std::pair<int, int> threshold) { threshold_ = threshold; }

  inline int get_min() { return out_.min; }
  inline int get_max() { return out_.max; }
  inline int get_neutral() { return out_neutral_; }

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
  double xy_angle_;
  double xz_angle_;
  double yz_angle_;

  // Output data bounds.
  Bounds out_;
  int out_neutral_;

  // Digital joystick activation thresholds (negative, positive).
  std::pair<int, int> threshold_;

  // A buffer holding the current joystick coordinates that can be used when the
  // sensor data isn't ready yet.
  Coordinates curr_coords_;

  // Time since the last sensor data fetch.
  unsigned long last_fetch_micros_;
};

}  // namespace hs

#endif  // HALL_JOYSTICK_H_
