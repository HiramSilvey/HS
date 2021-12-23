// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <memory>

#include "teensy.h"

class Configurator {
 public:
  static void Configure(std::unique_ptr<Teensy> teensy);

 private:
  static void FetchStoredBounds(const std::unique_ptr<Teensy>& teensy);
  static void FetchJoystickCoords(const std::unique_ptr<Teensy>& teensy);
  static void CalibrateJoystick(const std::unique_ptr<Teensy>& teensy);
  static void SaveCalibration(const std::unique_ptr<Teensy>& teensy);
  static void StoreProfiles(const std::unique_ptr<Teensy>& teensy);
};

#endif  // CONFIGURATOR_H_
