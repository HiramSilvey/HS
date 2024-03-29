// Copyright 2024 Hiram Silvey

#ifndef PINS_H_
#define PINS_H_

#include <vector>

#include "profile.pb.h"

namespace hs {
namespace pins {

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
const int kLeftOuter = 14;
const int kLeftInner = 15;

struct ActionPin {
  hs_profile_Profile_Layer_Action action;
  int pin;
};

// Get pins associated with each profile layer action.
std::vector<ActionPin> GetActionPins(const hs_profile_Profile_Layer& layer);

}  // namespace pins
}  // namespace hs

#endif  // PINS_H_
