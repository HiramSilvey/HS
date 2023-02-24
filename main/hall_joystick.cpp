// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <memory>

#include "math.h"
#include "pins.h"
#include "teensy.h"
#include "util.h"

namespace hs {

// Define teensy analogRead bit precision.
constexpr int kInputPrecision = 10;
constexpr int kOutputMin = 0;

HallJoystick::HallJoystick(Teensy& teensy, int out_precision, int threshold)
    : precision_diff_(kInputPrecision - out_precision),
      out_max_(teensy.Pow(2, out_precision) - 1),
      out_neutral_(out_max_ / 2),
      threshold_({threshold * -1, threshold}) {}

int HallJoystick::Translate(int val) {
  if (precision_diff_ >= 0) {
    return val >> precision_diff_;
  }
  return val << -precision_diff_;
}

int HallJoystick::ResolveDigitalCoord(int coord) {
  const int percent_tilted = ((coord - out_neutral_) * 100) / out_neutral_;
  if (percent_tilted <= threshold_.first) {
    return kOutputMin;
  }
  if (percent_tilted >= threshold_.second) {
    return out_max_;
  }
  return out_neutral_;
}

HallJoystick::Coordinates HallJoystick::GetCoordinates(Teensy& teensy) {
  int x = teensy.AnalogRead(pins::kX);
  int y = teensy.AnalogRead(pins::kY);

  x = Translate(x);
  y = Translate(y);

  if (threshold_.first < 0) {
    // DIGITAL
    return {ResolveDigitalCoord(x), ResolveDigitalCoord(y)};
  }

  return {x, y};
}

int HallJoystick::get_min() { return kOutputMin; }

int HallJoystick::get_max() { return out_max_; }

int HallJoystick::get_neutral() { return out_neutral_; }

}  // namespace hs
