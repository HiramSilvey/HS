// Copyright 2021 Hiram Silvey

#ifndef MAIN_JOYSTICK_H_
#define MAIN_JOYSTICK_H_

// Neutral, minimum, and maximum values each joystick axis is expected to
// output.
struct JoystickBounds {
  int neutral;
  int min;
  int max;
};

class Joystick {
 public:
  explicit Joystick(JoystickBounds bounds)
    : bounds_(bounds), sensor_(Tlv493d()) {}

  // Initializations to be run once before the main loop.
  void Setup();

  // Read and return X axis value.
  int GetX();

  // Read and return Y axis value.
  int GetY();

 private:
  const JoystickBounds bounds_;
  Tlv493d sensor_;
}

#endif  // MAIN_JOYSTICK_H_
