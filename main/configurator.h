// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <Tlv493d.h>

class Configurator {
public:
  Configurator();
  void Configure();
private:
  void FetchJoystickCoords();
  void CalibrateJoystick();
  void SaveCalibration();
  void StoreProfiles();

  Tlv493d sensor_;
};

#endif  // CONFIGURATOR_H_
