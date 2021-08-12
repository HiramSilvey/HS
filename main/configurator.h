// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <Tlv493d.h>

class Configurator {
public:
  static void Configure();
private:
  static void FetchStoredBounds();
  static void FetchJoystickCoords(Tlv493d& sensor);
  static void CalibrateJoystick(Tlv493d& sensor);
  static void SaveCalibration();
  static void StoreProfiles();
};

#endif  // CONFIGURATOR_H_
