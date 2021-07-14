// Copyright 2021 Hiram Silvey

#ifndef PC_CONTROLLER_H_
#define PC_CONTROLLER_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include "controller.h"
#include "hall_joystick.h"

class PCController: public Controller {
public:
  PCController();
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
  std::vector<int> hat_up_;
  std::vector<int> hat_down_;
  std::vector<int> hat_left_;
  std::vector<int> hat_right_;
};

#endif  // PC_CONTROLLER_H_
