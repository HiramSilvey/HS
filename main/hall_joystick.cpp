// Copyright 2021 Hiram Silvey

#include "hall_joystick.h"

#include <memory>

#include "math.h"
#include "teensy.h"
#include "util.h"

namespace hs {

HallJoystick::HallJoystick(const Teensy& teensy, int min, int max,
			   int threshold)
    : out_({.min = min, .neutral = (max - min + 1) / 2 + min, .max = max}),
      threshold_({threshold * -1, threshold}),
      last_fetch_micros_(0) {
  const int neutral_x = util::ReadIntFromEEPROM(teensy, 0);
  const int neutral_y = util::ReadIntFromEEPROM(teensy, 4);
  const int range = util::ReadIntFromEEPROM(teensy, 8);
  set_x_in(neutral_x, range);
  set_y_in(neutral_y, range);
  set_xy_angle(util::ReadShortFromEEPROM(teensy, 12));
  set_xz_angle(util::ReadShortFromEEPROM(teensy, 14));
  set_yz_angle(util::ReadShortFromEEPROM(teensy, 16));
  curr_coords_ = {out_.neutral, out_.neutral};
}

HallJoystick::Point HallJoystick::RotatePoint(const HallJoystick::Point& p) {
  // 1. XY plane rotation.
  const double r1_x = p.x * cos(xy_angle_) + p.y * sin(xy_angle_);
  const double r1_y = -p.x * sin(xy_angle_) + p.y * cos(xy_angle_);
  // 2. XZ plane rotation.
  const double r2_x = r1_x * cos(xz_angle_) - p.z * sin(xz_angle_);
  const double r2_z = r1_x * sin(xz_angle_) + p.z * cos(xz_angle_);
  // 3. YZ axis rotation.
  const double r3_y = r1_y * cos(yz_angle_) + r2_z * sin(yz_angle_);
  const double r3_z = -r1_y * sin(yz_angle_) + r2_z * cos(yz_angle_);

  return {r2_x, r3_y, r3_z};
}

int HallJoystick::Normalize(const Teensy& teensy, int val,
			    const InputBounds& in) {
  int mapped =
      round(static_cast<double>(val - in.min) /
		static_cast<double>(in.max - in.min) * (out_.max - out_.min) +
	    out_.min);
  return teensy.Constrain(mapped, out_.min, out_.max);
}

int HallJoystick::ResolveDigitalCoord(int coord) {
  const int percent_tilted =
      ((coord - out_.neutral) * 100) / (out_.neutral - out_.min);
  if (percent_tilted <= threshold_.first) {
    return out_.min;
  }
  if (percent_tilted >= threshold_.second) {
    return out_.max;
  }
  return out_.neutral;
}

HallJoystick::Coordinates HallJoystick::GetCoordinates(Teensy& teensy) {
  if (teensy.Micros() - last_fetch_micros_ < 330) {
    return curr_coords_;
  }

  teensy.UpdateHallData();
  last_fetch_micros_ = teensy.Micros();

  HallJoystick::Point cursor = {teensy.GetHallX(), teensy.GetHallY(),
				teensy.GetHallZ()};

  cursor = RotatePoint(cursor);

  // Flatten points onto the XY plane.
  int x = cursor.x / cursor.z * 1000000;
  int y = cursor.y / cursor.z * 1000000;

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
