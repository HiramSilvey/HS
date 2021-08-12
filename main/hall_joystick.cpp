// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <Tlv493d.h>
#include <EEPROM.h>
#include "util.h"

HallJoystick::HallJoystick(int min, int max, int threshold)
  : out_({.min = min, .max = max}),
    out_neutral_((max - min + 1) / 2 + min),
    threshold_({threshold * -1, threshold}) {
  int neutral_x = Util::GetIntFromEEPROM(0);
  int neutral_y = Util::GetIntFromEEPROM(4);
  int range = Util::GetIntFromEEPROM(8);
  x_in_ = {.min = neutral_x - range, .max = neutral_x + range};
  y_in_ = {.min = neutral_y - range, .max = neutral_y + range};
}

void HallJoystick::Init() {
  sensor_.begin();
  sensor_.setAccessMode(sensor_.LOWPOWERMODE);
  sensor_.disableTemp();
}

int HallJoystick::Normalize(int val, const Bounds& in) {
  int mapped = static_cast<float>(val - in.min) /
    static_cast<float>(in.max - in.min) * (out_.max - out_.min) + out_.min;
  return constrain(mapped, out_.min, out_.max);
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

HallJoystick::Coordinates HallJoystick::GetCoordinates() {
  sensor_.updateData();
  float z = sensor_.getZ();
  int x = sensor_.getX() / z * 1000000;
  int y = sensor_.getY() / z * 1000000;

  x = Normalize(x, x_in_);
  y = Normalize(y, y_in_);

  if (threshold_.first < 0) {
    // DIGITAL
    return {ResolveDigitalCoord(x), ResolveDigitalCoord(y)};
  }
  // ANALOG
  return {x, y};
}

int HallJoystick::get_min() {
  return out_.min;
}

int HallJoystick::get_max() {
  return out_.max;
}

int HallJoystick::get_neutral() {
  return out_neutral_;
}
