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
  int ResolveSOCD(std::vector<AnalogButton> buttons);
  int GetDPadAngle();

  std::unique_ptr<HallJoystick> joystick_;
  std::unordered_map<int, std::vector<int>> button_id_to_pins_;
  std::vector<AnalogButton> z_y_;
  std::vector<AnalogButton> z_x_;
  std::vector<AnalogButton> slider_left_;
  std::vector<AnalogButton> slider_right_;
  int hat_up_;
  int hat_down_;
  int hat_left_;
  int hat_right_;
};

#endif  // USB_CONTROLLER_H_
