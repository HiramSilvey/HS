// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <memory>

#include "mcu.h"

class Configurator {
public:
  static void Configure(std::unique_ptr<MCU> mcu);
private:
  static void FetchStoredBounds(std::unique_ptr<MCU>& mcu);
  static void FetchJoystickCoords(std::unique_ptr<MCU>& mcu);
  static void CalibrateJoystick(std::unique_ptr<MCU>& mcu);
  static void SaveCalibration(std::unique_ptr<MCU>& mcu);
  static void StoreProfiles(std::unique_ptr<MCU>& mcu);
};

#endif  // CONFIGURATOR_H_
