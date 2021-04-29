// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <Tlv493d.h>

class HallJoystick {
 public:
  // Minimum, maximum, and neutral values each joystick axis is expected to
  // output.
  explicit HallJoystick(int min, int max, int neutral) {
    out_ = {
            .min = min,
            .max = max,
            .neutral = neutral
    };
  }

  // Initializations to be run once before the main loop.
  void Init();

  // Read and return X axis value.
  int GetX();

  // Read and return Y axis value.
  int GetY();

 private:
  struct Bounds {
    int min;
    int max;
    int neutral;
  };

  // Map the provided float value from the specified input range to the global
  // output range.
  int Normalize(float val, const Bounds& in);

  Bounds x_in_;
  Bounds y_in_;
  Bounds out_;
  Tlv493d sensor_;
};

#endif  // HALL_JOYSTICK_H_
