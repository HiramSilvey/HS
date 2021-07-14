// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <Tlv493d.h>

class Configurator {
public:
  static void Configure();
private:
  static void StoreProfiles();
  static void CalibrateJoystick();
};

#endif  // CONFIGURATOR_H_
