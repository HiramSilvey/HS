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
  void FetchJoystickCoords();
  void CalibrateJoystick();
  void SaveCalibration();
  void StoreProfiles();
  void IncXYAngle();
  void DecXYAngle();
  void IncXZAngle();
  void DecXZAngle();
  void IncYZAngle();
  void DecYZAngle();
  void WriteOk();

  std::unique_ptr<Teensy> teensy_;
  std::unique_ptr<HallJoystick> joystick_;
  int neutral_x_ = 0;
  int neutral_y_ = 0;
  int range_ = 0;
  int range_tick_ = 0;
  uint16_t xy_angle_ticks_ = 0;
  uint16_t xz_angle_ticks_ = 0;
  uint16_t yz_angle_ticks_ = 0;
};

}  // namespace hs

#endif  // CONFIGURATOR_H_
