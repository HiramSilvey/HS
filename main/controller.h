// Copyright 2021 Hiram Silvey

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <vector>

#include "profile.pb.h"

class Controller {
public:
  // Initializations to be run once before the main loop. Returns true on
  // success, false otherwise.
  virtual bool Init() = 0;

  // Main loop to be run each tick.
  virtual void Loop() = 0;

protected:
  struct AnalogButton {
    int value;
    int pin;
  };

  // Fetch the specified profile given the platform.
  static hs_profile_Profile_Layout FetchProfile(hs_profile_Profile_Platform Platform);

  // Resolve simultaneous opposing cardinal directions from button inputs.
  static int ResolveSOCD(std::vector<AnalogButton> buttons, int joystick_neutral);

private:
  // Load the controller profile settings based on the button held.
  virtual void LoadProfile() = 0;
};

#endif  // CONTROLLER_H_
