// Copyright 2021 Hiram Silvey

#ifndef USB_CONTROLLER_H_
#define USB_CONTROLLER_H_

#include <unordered_map>
#include <vector>
#include "controller.h"

class USBController: public Controller {
public:
  USBController();
  bool Init() override;
  void Loop() override;

private:
  struct AnalogButton {
    int value;
    int pin;
  };

  void LoadProfile() override;
  int GetDPadAngle();

  std::unique_ptr<HallJoystick> joystick_;
  std::unordered_map<int, std::vector<int>> button_id_to_pins_;
  AnalogButton z_up_;
  AnalogButton z_down_;
  AnalogButton z_left_;
  AnalogButton z_right_;
  AnalogButton slider_left_low_;
  AnalogButton slider_left_high_;
  AnalogButton slider_right_low_;
  AnalogButton slider_right_high_;
  int hat_up_;
  int hat_down_;
  int hat_left_;
  int hat_right_;
};

#endif  // USB_CONTROLLER_H_
