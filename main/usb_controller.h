// Copyright 2021 Hiram Silvey

#ifndef USB_CONTROLLER_H_
#define USB_CONTROLLER_H_

#include <memory>
#include <unordered_map>

#include "controller.h"
#include "hall_joystick.h"

class USBController: public Controller {
 public:
  USBController();
  bool Init() override;
  void Loop() override;

 private:
  int ResolveSOCD(int low_direction, int high_direction) override;

  std::unique_ptr<HallJoystick> joystick_;
  std::unordered_map<int, int> layout_;
};

#endif  // USB_CONTROLLER_H_
