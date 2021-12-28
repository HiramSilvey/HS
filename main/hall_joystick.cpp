// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <memory>

#include "teensy.h"
#include "util.h"

namespace hs {

HallJoystick::HallJoystick(const Teensy& teensy, int min, int max,
                           int threshold)
    : out_({.min = min, .max = max}),
      out_neutral_((max - min + 1) / 2 + min),
      threshold_({threshold * -1, threshold}) {
  int neutral_x = util::GetIntFromEEPROM(teensy, 0);
  int neutral_y = util::GetIntFromEEPROM(teensy, 4);
  int range = util::GetIntFromEEPROM(teensy, 8);
  x_in_ = {.min = neutral_x - range, .max = neutral_x + range};
  y_in_ = {.min = neutral_y - range, .max = neutral_y + range};
}

int HallJoystick::Normalize(const Teensy& teensy, int val, const Bounds& in) {
  int mapped = static_cast<float>(val - in.min) /
                   static_cast<float>(in.max - in.min) * (out_.max - out_.min) +
               out_.min;
  return teensy.Constrain(mapped, out_.min, out_.max);
}

int HallJoystick::ResolveDigitalCoord(int coord) {
  const int percent_tilted = ((coord - out_neutral_) * 100) / out_neutral_;
  if (percent_tilted <= threshold_.first) {
    return out_.min;
  }
  if (percent_tilted >= threshold_.second) {
    return out_.max;
  }
  return out_neutral_;
}

HallJoystick::Coordinates HallJoystick::GetCoordinates(Teensy& teensy) {
  teensy.UpdateHallData();
  float z = teensy.GetHallZ();
  int x = teensy.GetHallX() / z * 1000000;
  int y = teensy.GetHallY() / z * 1000000;

  x = Normalize(teensy, x, x_in_);
  y = Normalize(teensy, y, y_in_);

  if (threshold_.first < 0) {
    // DIGITAL
    return {ResolveDigitalCoord(x), ResolveDigitalCoord(y)};
  }
  // ANALOG
  return {x, y};
}

int HallJoystick::get_min() { return out_.min; }

int HallJoystick::get_max() { return out_.max; }

int HallJoystick::get_neutral() { return out_neutral_; }

}  // namespace hs
