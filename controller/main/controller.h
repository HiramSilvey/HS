// Copyright 2021 Hiram Silvey

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <memory>

#include "hall_joystick.h"

class Controller {
public:
  // Initializations to be run once before the main loop. Returns true on
  // success, false otherwise.
  virtual bool Init() = 0;

  // Main loop to be run each tick.
  virtual void Loop() = 0;

protected:
  static void InitPins();

  // Simultaneous opposing cardinal direction resolution.
  static int ResolveSOCD(int low_direction, int high_direction, int joystick_min,
                         int joystick_max, int joystick_neutral);

private:
  // Load the controller profile settings based on the button held.
  virtual void LoadProfile() = 0;
};

#endif  // CONTROLLER_H_
