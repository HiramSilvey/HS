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
      threshold_({threshold * -1, threshold}),
      last_fetch_micros_(0) {
  int neutral_x = util::ReadIntFromEEPROM(teensy, 0);
  int neutral_y = util::ReadIntFromEEPROM(teensy, 4);
  int range = util::ReadIntFromEEPROM(teensy, 8);
  set_x_in(neutral_x, range);
  set_y_in(neutral_y, range);
  set_xy_angle(util::ReadShortFromEEPROM(teensy, 12));
  set_xz_angle(util::ReadShortFromEEPROM(teensy, 14));
  set_yz_angle(util::ReadShortFromEEPROM(teensy, 16));
  curr_coords_ = {out_neutral_, out_neutral_};
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

HallJoystick::Coordinates HallJoystick::GetCoordinates(Teensy& teensy) {
  if (teensy.Micros() - last_fetch_micros_ < 330) {
    return curr_coords_;
  }

  teensy.UpdateHallData();
  last_fetch_micros_ = teensy.Micros();

  float raw_x = teensy.GetHallX();
  float raw_y = -teensy.GetHallY();
  float raw_z = teensy.GetHallZ();

  // Apply rotations piecewise. No-op if the angle is 0.
  // 1. XY plane rotation.
  double rotated_x = raw_x * cos(xy_angle_) + raw_y * sin(xy_angle_);
  double rotated_y = -raw_x * sin(xy_angle_) + raw_y * cos(xy_angle_);
  // 2. XZ plane rotation.
  rotated_x = rotated_x * cos(xz_angle_) - raw_z * sin(xz_angle_);
  double rotated_z = rotated_x * sin(xz_angle_) + raw_z * cos(xz_angle_);
  // 3. YZ axis rotation.
  rotated_y = rotated_y * cos(yz_angle_) + rotated_z * sin(yz_angle_);
  rotated_z = -rotated_y * sin(yz_angle_) + rotated_z * cos(yz_angle_);

  // Flatten points onto the XY plane.
  int x = rotated_x / rotated_z * 1000000;
  int y = rotated_y / rotated_z * 1000000;

  x = Normalize(teensy, x, x_in_);
  y = Normalize(teensy, y, y_in_);

  if (threshold_.first < 0) {
    // DIGITAL
    curr_coords_ = {ResolveDigitalCoord(x), ResolveDigitalCoord(y)};
  } else {
    // ANALOG
    curr_coords_ = {x, y};
  }
  return curr_coords_;
}

}  // namespace hs
