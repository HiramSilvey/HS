// Copyright 2021 Hiram Silvey

#ifndef USB_CONTROLLER_H_
#define USB_CONTROLLER_H_

#include <unordered_map>

#include "controller.h"

class USBController: public Controller {
public:
  USBController();
  bool Init() override;
  void Loop() override;

private:
  void LoadProfile() override;

  std::unique_ptr<HallJoystick> joystick_;
  std::unordered_map<int, int> layout_;
};

#endif  // USB_CONTROLLER_H_
