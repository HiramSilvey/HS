// Copyright 2021 Hiram Silvey

#ifndef NS_CONTROLLER_H_
#define NS_CONTROLLER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "controller.h"
#include "hall_joystick.h"
#include "nspad.h"
#include "teensy.h"

namespace hs {

struct NSButtonPinMapping : ButtonPinMapping {
  std::vector<AnalogButton> z_y;
  std::vector<AnalogButton> z_x;
  std::vector<int> dpad_up;
  std::vector<int> dpad_down;
  std::vector<int> dpad_left;
  std::vector<int> dpad_right;
};

class NSController : public Controller {
 public:
  NSController(std::unique_ptr<Teensy> teensy, std::unique_ptr<NSPad> nspad);
  NSButtonPinMapping GetButtonPinMapping(const hs_profile_Profile_Layer& layer);
  void LoadProfile() override;
  int GetDPadDirection(const NSButtonPinMapping& mapping);
  void UpdateButtons(const NSButtonPinMapping& mapping);
  void Loop() override;

 private:
  std::unique_ptr<Teensy> teensy_;
  std::unique_ptr<NSPad> nspad_;
  std::unique_ptr<HallJoystick> joystick_;
  int dpad_direction_[16];
  NSButtonPinMapping base_mapping_;
  NSButtonPinMapping mod_mapping_;
};

}  // namespace hs

#endif  // NS_CONTROLLER_H_
