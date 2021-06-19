// Copyright 2021 Hiram Silvey

#ifndef PINS_H_
#define PINS_H_

class Pins {
public:
  struct ActionPin {
    hs_profile_Profile_Layout_Action action;
    int pin;
  };

  // Initialize all button pins.
  static void Init();

  // Get pins associated with each profile layout action.
  static std::vector<ActionPin> GetActionPins(const hs_profile_Profile_Layout& layout);
}

#endif  // PINS_H_
