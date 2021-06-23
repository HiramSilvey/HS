// Copyright 2021 Hiram Silvey

#ifndef PINS_H_
#define PINS_H_

#include <vector>
#include "profile.pb.h"

// Button pins.
const int kThumbTop = 0;
const int kThumbMiddle = 1;
const int kThumbBottom = 2;
const int kIndexTop = 3;
const int kIndexMiddle = 4;
const int kMiddleTop = 5;
const int kMiddleMiddle = 6;
const int kMiddleBottom = 7;
const int kRingTop = 8;
const int kRingMiddle = 9;
const int kRingBottom = 10;
const int kPinkyTop = 11;
const int kPinkyMiddle = 12;
const int kPinkyBottom = 13;
const int kLeftRingExtra = 14;
const int kLeftMiddleExtra = 15;
const int kLeftIndexExtra = 16;
const int kRightIndexExtra = 17;
const int kRightMiddleExtra = 20;
const int kRightRingExtra = 21;

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
};

#endif  // PINS_H_
