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
  struct ButtonPinMapping {
    std::unordered_map<int, std::vector<int>> button_id_to_pins;
    std::vector<Controller::AnalogButton> z_y;
    std::vector<Controller::AnalogButton> z_x;
    std::vector<Controller::AnalogButton> slider_left;
    std::vector<Controller::AnalogButton> slider_right;
    std::vector<int> hat_up;
    std::vector<int> hat_down;
    std::vector<int> hat_left;
    std::vector<int> hat_right;
    std::vector<int> mod;
  };

  ButtonPinMapping GetButtonPinMapping(const hs_profile_Profile_Layer& layer);
  void LoadProfile() override;
  int GetDPadAngle(const ButtonPinMapping& mapping);
  void UpdateButtons(const ButtonPinMapping& mapping);

  std::unique_ptr<HallJoystick> joystick_;
  ButtonPinMapping base_mapping_;
  ButtonPinMapping mod_mapping_;
};

#endif  // PC_CONTROLLER_H_
