// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <memory>

#include "hall_joystick.h"
#include "teensy.h"

namespace hs {

class Configurator {
 public:
  explicit Configurator(std::unique_ptr<Teensy> teensy);
  void Loop();

 private:
  void IncNeutralX();
  void DecNeutralX();
  void IncNeutralY();
  void DecNeutralY();
  void IncRange();
  void DecRange();
  void IncXYAngle();
  void DecXYAngle();
  void IncXZAngle();
  void DecXZAngle();
  void IncYZAngle();
  void DecYZAngle();

  void FetchJoystickCoords();
  void CalibrateJoystick();
  void SaveCalibration();
  void StoreProfiles();

  void FetchJoystickState();

  std::unique_ptr<Teensy> teensy_;
  std::unique_ptr<HallJoystick> joystick_;
  HallJoystick::Point neutral_x_;
  HallJoystick::Point neutral_y_;
  int range_ = 0;
  int range_tick_ = 0;
  int16_t xy_angle_ticks_ = 0;
  int16_t xz_angle_ticks_ = 0;
  int16_t yz_angle_ticks_ = 0;
};

}  // namespace hs

#endif  // CONFIGURATOR_H_
