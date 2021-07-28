// Copyright 2021 Hiram Silvey

#ifndef NS_CONTROLLER_H_
#define NS_CONTROLLER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "controller.h"
#include "hall_joystick.h"

class NSController: public Controller {
public:
  NSController();
  bool Init() override;
  void Loop() override;

private:
  void LoadProfile() override;
  int GetDPadDirection();

  std::unique_ptr<HallJoystick> joystick_;
  std::unordered_map<int, std::vector<int>> button_id_to_pins_;
  std::vector<Controller::AnalogButton> z_y_;
  std::vector<Controller::AnalogButton> z_x_;
  std::vector<int> dpad_up_;
  std::vector<int> dpad_down_;
  std::vector<int> dpad_left_;
  std::vector<int> dpad_right_;
};

#endif  // NS_CONTROLLER_H_
