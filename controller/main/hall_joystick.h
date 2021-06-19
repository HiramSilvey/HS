// Copyright 2021 Hiram Silvey

#ifndef HALL_JOYSTICK_H_
#define HALL_JOYSTICK_H_

#include <Tlv493d.h>

class HallJoystick {
public:
  // Minimum and maximum values each joystick axis is expected to output.
  explicit HallJoystick(int min, int max);

  int GetMin();
  int GetMax();
  int GetNeutral();

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
