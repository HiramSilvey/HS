// Copyright 2021 Hiram Silvey

#ifndef MAIN_USB_CONTROLLER_H_
#define MAIN_USB_CONTROLLER_H_

#include "main/controller.h"
#include "main/hall_joystick.h"

class USBController: public Controller {
 public:
  USBController();

 private:
  HallJoystick joystick_;
}

#endif  // MAIN_USB_CONTROLLER_H_
