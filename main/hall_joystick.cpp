// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <memory>

#include "math.h"
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
  int16_t angle_ticks = util::GetShortFromEEPROM(teensy, 12);
  angle_ = (M_PI * angle_ticks) / 2000.0;
  analog_coords_ = {out_neutral_, out_neutral_};
  digital_coords_ = {out_neutral_, out_neutral_};
}

int HallJoystick::Normalize(const Teensy& teensy, double val,
			    const Bounds& in) {
  int mapped =
      round(static_cast<double>(val - in.min) /
		static_cast<double>(in.max - in.min) * (out_.max - out_.min) +
	    out_.min);
  return teensy.Constrain(mapped, out_.min, out_.max);
}

int HallJoystick::ResolveDigitalCoord(int coord) {
  const int percent_tilted =
      ((coord - out_neutral_) * 100) / (out_neutral_ - out_.min);
  if (percent_tilted <= threshold_.first) {
    return out_.min;
  }
  if (percent_tilted >= threshold_.second) {
    return out_.max;
  }
  return out_neutral_;
}

  HallJoystick::Coordinates HallJoystick::FilterSnapback(int x, int y, long delta_t) {
    const double delta_x = x - analog_coords_.x;
    const double delta_y = y - analog_coords_.y;
    const double distance = sqrt(pow(delta_x, 2) + pow(delta_y, 2));
    curr_distance_ =
	(curr_distance_ - distance) * exp(-frequency_cutoff_ * delta_t) +
	distance;

    const double total_delta = delta_x + delta_y;
    return {(delta_x / total_delta) * curr_distance_,
	    (delta_y / total_delta) * curr_distance_};
  }

  HallJoystick::Coordinates HallJoystick::CachedCoordinates() {

  }

HallJoystick::Coordinates HallJoystick::GetCoordinates(Teensy& teensy) {
  if (!teensy.HallDataAvailable()) {
    if (threshold_.first < 0) {
      return digital_coords_;
    }
    return analog_coords_;
  }

  const float z = teensy.GetHallZ();
  int x = teensy.GetHallX() / z * 1000000;
  int y = teensy.GetHallY() / z * 1000000;

  // Rotate the coordinates according to the configuration angle. This is a
  // no-op if the angle is 0.
  double rotated_x = x * cos(angle_) + y * sin(angle_);
  double rotated_y = -x * sin(angle_) + y * cos(angle_);

  x = Normalize(teensy, rotated_x, x_in_);
  y = Normalize(teensy, rotated_y, y_in_);

  analog_coords_ = FilterSnapback(x, y);

  if (threshold_.first < 0) {
    digital_coords_ = {ResolveDigitalCoord(x), ResolveDigitalCoord(y)};
    return digital_coords_;
  }
  return analog_coords_;
}

int HallJoystick::get_min() { return out_.min; }

int HallJoystick::get_max() { return out_.max; }

int HallJoystick::get_neutral() { return out_neutral_; }

}  // namespace hs
