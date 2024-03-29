// Copyright 2024 Hiram Silvey

#ifndef PC_CONTROLLER_H_
#define PC_CONTROLLER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "controller.h"
#include "hall_joystick.h"
#include "teensy.h"

namespace hs {

struct PCButtonPinMapping : ButtonPinMapping {
  std::vector<AnalogButton> z_y;
  std::vector<AnalogButton> z_x;
  std::vector<AnalogButton> slider_left;
  std::vector<AnalogButton> slider_right;
  std::vector<int> hat_up;
  std::vector<int> hat_down;
  std::vector<int> hat_left;
  std::vector<int> hat_right;
};

class PCController : public Controller {
 public:
  PCController(std::unique_ptr<Teensy> teensy);
  PCButtonPinMapping GetButtonPinMapping(const hs_profile_Profile_Layer& layer);
  void LoadProfile() override;
  int GetDPadAngle(const PCButtonPinMapping& mapping);
  void UpdateButtons(const PCButtonPinMapping& mapping);
  void Loop() override;

 private:
  std::unique_ptr<Teensy> teensy_;
  std::unique_ptr<HallJoystick> joystick_;
  PCButtonPinMapping base_mapping_;
  PCButtonPinMapping mod_mapping_;
};

#endif  // PC_CONTROLLER_H_

}  // namespace hs
