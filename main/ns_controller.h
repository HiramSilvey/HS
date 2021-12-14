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
  struct ButtonPinMapping {
    std::unordered_map<int, std::vector<int>> button_id_to_pins;
    std::vector<Controller::AnalogButton> z_y;
    std::vector<Controller::AnalogButton> z_x;
    std::vector<int> dpad_up;
    std::vector<int> dpad_down;
    std::vector<int> dpad_left;
    std::vector<int> dpad_right;
    std::vector<int> mod;
  };

  ButtonPinMapping GetButtonPinMapping(const hs_profile_Profile_Layer& layer);
  void LoadProfile() override;
  int GetDPadDirection(const ButtonPinMapping& mapping);
  void UpdateButtons(const ButtonPinMapping& mapping);

  std::unique_ptr<HallJoystick> joystick_;
  ButtonPinMapping base_mapping_;
  ButtonPinMapping mod_mapping_;
};

#endif  // NS_CONTROLLER_H_
