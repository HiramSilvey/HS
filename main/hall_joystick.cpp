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
  z_min_ = util::ReadIntFromEEPROM(teensy, 12);
  set_xy_angle(util::ReadShortFromEEPROM(teensy, 16));
  set_xz_angle(util::ReadShortFromEEPROM(teensy, 18));
  set_yz_angle(util::ReadShortFromEEPROM(teensy, 20));
  curr_coords_ = {out_.neutral, out_.neutral};
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
      ((coord - out_.neutral) * 100) / (out_.neutral - out_.min);
  if (percent_tilted <= threshold_.first) {
    return out_.min;
  }
  if (percent_tilted >= threshold_.second) {
    return out_.max;
  }
  return out_.neutral;
}

HallJoystick::Point HallJoystick::RotatePoint(const HallJoystick::Point& p) {
  // Translate neutral inputs to the origin.
  const float x = p.x - x_in_.neutral;
  const float y = p.y - y_in_.neutral;
  const float z = p.z - z_min_;

  // 1. XY plane rotation.
  const double r1_x = x * cos(xy_angle_) + y * sin(xy_angle_);
  const double r1_y = -x * sin(xy_angle_) + y * cos(xy_angle_);
  // 2. XZ plane rotation.
  const double r2_x = r1_x * cos(xz_angle_) - z * sin(xz_angle_);
  const double r2_z = r1_x * sin(xz_angle_) + z * cos(xz_angle_);
  // 3. YZ axis rotation.
  const double r3_y = r1_y * cos(yz_angle_) + r2_z * sin(yz_angle_);
  const double r3_z = -r1_y * sin(yz_angle_) + r2_z * cos(yz_angle_);

  // Translate back to original position.
  return {
      .x = r2_x + x_in_.neutral, .y = r3_y + y_in_.neutral, .z = r3_z + z_min_};
}

HallJoystick::Coordinates HallJoystick::GetCoordinates(Teensy& teensy) {
  if (teensy.Micros() - last_fetch_micros_ < 330) {
    return curr_coords_;
  }

  teensy.UpdateHallData();
  last_fetch_micros_ = teensy.Micros();

  const int raw_x = teensy.GetHallX() * 1000000;
  const int raw_y = -teensy.GetHallY() * 1000000;
  const int raw_z = teensy.GetHallZ() * 1000000;

  // TODO: log raw z to determine whether min or max should be stored

  double transformed_x = raw_x;
  double transformed_y = raw_y;
  double transformed_z = raw_z;

  if (xy_angle_ != 0 || xz_angle_ != 0 || yz_angle_ != 0) {
  }

  // Flatten points onto the XY plane.
  int x = transformed_x / transformed_z * 1000000;
  int y = transformed_y / transformed_z * 1000000;

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
