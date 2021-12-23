// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <memory>

#include "mcu.h"

class Configurator {
public:
  static void Configure(std::unique_ptr<MCU> mcu);
private:
  static void FetchStoredBounds(const std::unique_ptr<MCU>& mcu);
  static void FetchJoystickCoords(const std::unique_ptr<MCU>& mcu);
  static void CalibrateJoystick(const std::unique_ptr<MCU>& mcu);
  static void SaveCalibration(const std::unique_ptr<MCU>& mcu);
  static void StoreProfiles(const std::unique_ptr<MCU>& mcu);
};

#endif  // CONFIGURATOR_H_
