// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <Tlv493d.h>

class HallJoystick {
 public:
  explicit HallJoystick(int neutral, int min, int max) {
    bounds_.neutral = neutral;
    bounds_.min = min;
    bounds_.max = max;
  }

  // Initializations to be run once before the main loop.
  void Init();

  // Read and return X axis value.
  int GetX();

  // Read and return Y axis value.
  int GetY();

 private:
  // Neutral, minimum, and maximum values each joystick axis is expected to
  // output.
  struct JoystickBounds {
    int neutral;
    int min;
    int max;
  };

  JoystickBounds bounds_;
  Tlv493d sensor_;
};

#endif  // HALL_JOYSTICK_H_
