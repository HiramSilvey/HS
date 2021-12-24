// Copyright 2021 Hiram Silvey

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "profile.pb.h"
#include "teensy.h"

struct AnalogButton {
  int value;
  int pin;
};

struct ButtonPinMapping {
  std::unordered_map<int, std::vector<int>> button_id_to_pins;
  std::vector<int> mod;
};

class Controller {
 public:
  // Fetch the specified profile given the platform.
  static hs_profile_Profile_Layout FetchProfile(
      const std::unique_ptr<Teensy>& teensy,
      const hs_profile_Profile_Platform& Platform);

  // Resolve simultaneous opposing cardinal directions from button inputs.
  static int ResolveSOCD(const std::unique_ptr<Teensy>& teensy,
                         const std::vector<AnalogButton>& buttons,
                         int joystick_neutral);

  // Main loop to be run each tick.
  virtual void Loop() = 0;

 private:
  // Load the controller profile settings based on the button held.
  virtual void LoadProfile() = 0;
};

#endif  // CONTROLLER_H_
