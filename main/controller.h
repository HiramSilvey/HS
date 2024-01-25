// Copyright 2024 Hiram Silvey

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <unordered_map>
#include <vector>

#include "profile.pb.h"
#include "teensy.h"

namespace hs {

struct AnalogButton {
  int value;
  int pin;
};

struct ButtonPinMapping {
  std::unordered_map<int, std::vector<int>> button_id_to_pins;
  std::vector<int> mod;
};

// Fetch the specified profile given the platform.
hs_profile_Profile_Layout FetchProfile(
    const Teensy& teensy, const hs_profile_Profile_Platform& Platform);

// Resolve simultaneous opposing cardinal directions from button inputs.
int ResolveSOCD(const Teensy& teensy, const std::vector<AnalogButton>& buttons,
                int joystick_neutral);

class Controller {
 public:
  // Load the controller profile settings based on the button held.
  virtual void LoadProfile() = 0;

  // Main loop to be run each tick.
  virtual void Loop() = 0;
};

}  // namespace hs

#endif  // CONTROLLER_H_
