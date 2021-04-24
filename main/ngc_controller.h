// Copyright 2021 Hiram Silvey

#ifndef MAIN_NGC_CONTROLLER_H_
#define MAIN_NGC_CONTROLLER_H_

#include <GamecubeAPI.h>
#include "main/controller.h"
#include "main/hall_joystick.h"

class NGCController: public Controller {
 public:
  NGCController();

 private:
  Gamecube_Data_t output_;
  CGamecubeController controller_;
  CGamecubeConsole console_;
  HallJoystick joystick_;
}

#endif  // MAIN_NGC_CONTROLLER_H_
