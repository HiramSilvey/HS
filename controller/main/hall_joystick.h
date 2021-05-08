// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <Tlv493d.h>

class HallJoystick {
public:
  // Minimum and maximum values each joystick axis is expected to output.
  explicit HallJoystick(int min, int max)
    : x_in_({.min = GetIntFromEEPROM(0), .max = GetIntFromEEPROM(4)}),
      y_in_({.min = GetIntFromEEPROM(8), .max = GetIntFromEEPROM(12)}),
      out_({.min = min, .max = max}),
      out_neutral_((max - min + 1) / 2 + min) {}

  int get_min() {
    return out_.min;
  }

  int get_max() {
    return out_.max;
  }

  int get_neutral() {
    return out_neutral_;
  }

  // Initializations to be run once before the main loop.
  void Init();

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

  // Read and return 4 consecutive bytes as an int from EEPROM, with the highest
  // order byte at the lowest address.
  int GetIntFromEEPROM(int address);

  // Input data bounds.
  const Bounds x_in_;
  const Bounds y_in_;

  // Output data bounds.
  const Bounds out_;
  const int out_neutral_;

  Tlv493d sensor_;
};

#endif  // HALL_JOYSTICK_H_
