// Copyright 2021 Hiram Silvey

#ifndef NS_CONTROLLER_H_
#define NS_CONTROLLER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "controller.h"
#include "hall_joystick.h"
#include "mcu.h"
#include "nspad.h"

class NSController: public Controller {
public:
  NSController(std::unique_ptr<MCU> mcu, std::unique_ptr<NSPad> nsgamepad);
  static bool Active();
  void Loop() override;

private:
  struct NSButtonPinMapping : Controller::ButtonPinMapping {
    std::vector<Controller::AnalogButton> z_y;
    std::vector<Controller::AnalogButton> z_x;
    std::vector<int> dpad_up;
    std::vector<int> dpad_down;
    std::vector<int> dpad_left;
    std::vector<int> dpad_right;
  };

  NSButtonPinMapping GetButtonPinMapping(const hs_profile_Profile_Layer& layer);
  void LoadProfile() override;
  int GetDPadDirection(const NSButtonPinMapping& mapping);
  void UpdateButtons(const NSButtonPinMapping& mapping);

  std::unique_ptr<MCU> mcu_;
  std::unique_ptr<NSPad> nsgamepad_;
  std::unique_ptr<HallJoystick> joystick_;
  int dpad_direction_[16];
  NSButtonPinMapping base_mapping_;
  NSButtonPinMapping mod_mapping_;
};

#endif  // NS_CONTROLLER_H_
