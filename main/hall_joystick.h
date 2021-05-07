// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <Tlv493d.h>

class HallJoystick {
 public:
  // Minimum and maximum values each joystick axis is expected to output.
  explicit HallJoystick(int min, int max) {
    out_ = {
            .min = min,
            .max = max
    };
  }

  struct Coordinates {
    int x;
    int y;
  };

  // Initializations to be run once before the main loop.
  void Init();

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

  // Read and return 4 consecutive bytes as an int from EEPROM, with the highest
  // order byte at the lowest address.
  int GetIntFromEEPROM(int address);

  Bounds x_in_;
  Bounds y_in_;
  Bounds out_;
  Tlv493d sensor_;
};

#endif  // HALL_JOYSTICK_H_
