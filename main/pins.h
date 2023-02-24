// Copyright 2021 Hiram Silvey

#ifndef PINS_H_
#define PINS_H_

#include <vector>

#include "profile.pb.h"

namespace hs {
namespace pins {

// Button pins.
constexpr int kThumbTop = 0;
constexpr int kThumbMiddle = 1;
constexpr int kThumbBottom = 2;
constexpr int kIndexTop = 3;
constexpr int kIndexMiddle = 4;
constexpr int kMiddleTop = 5;
constexpr int kMiddleMiddle = 6;
constexpr int kMiddleBottom = 7;
constexpr int kRingTop = 8;
constexpr int kRingMiddle = 9;
constexpr int kRingBottom = 10;
constexpr int kPinkyTop = 11;
constexpr int kPinkyMiddle = 12;
constexpr int kPinkyBottom = 13;
constexpr int kLeftOuter = 14;
constexpr int kLeftInner = 15;

// Joystick pins.
constexpr int kX = 17;
constexpr int kY = 19;

struct ActionPin {
  hs_profile_Profile_Layer_Action action;
  int pin;
};

// Get pins associated with each profile layer action.
std::vector<ActionPin> GetActionPins(const hs_profile_Profile_Layer& layer);

}  // namespace pins
}  // namespace hs

#endif  // PINS_H_
