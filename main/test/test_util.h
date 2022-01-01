#ifndef TEST_UTIL_H_
#define TEST_UTIL_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "controller.h"
#include "profile.pb.h"

namespace hs {

using ::testing::AllOf;
using ::testing::Field;
using ::testing::Matcher;

hs_profile_Profile_Layer_Action DigitalLayerAction(
    hs_profile_Profile_Layer_DigitalAction action) {
  return hs_profile_Profile_Layer_Action{
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
      .action_type = {.digital = action}};
}

hs_profile_Profile_Layer_Action AnalogLayerAction(
    hs_profile_Profile_Layer_AnalogAction_ID id, int val) {
  return hs_profile_Profile_Layer_Action{
      .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
      .action_type = {.analog = {.id = id, .value = val}}};
}

MATCHER_P(ActionTypeEq, expected, "") {
  return arg.action_type.digital == expected.digital &&
         arg.action_type.analog.id == expected.analog.id &&
         arg.action_type.analog.value == expected.analog.value;
}

auto ActionEq(const hs_profile_Profile_Layer_Action& expected) {
  return AllOf(Field(&hs_profile_Profile_Layer_Action::which_action_type,
                     expected.which_action_type),
               ActionTypeEq(expected.action_type));
}

std::vector<Matcher<AnalogButton>> AnalogEq(
    const std::vector<AnalogButton>& expected) {
  std::vector<Matcher<AnalogButton>> matchers;
  for (const auto& button : expected) {
    matchers.push_back(AllOf(Field(&AnalogButton::value, button.value),
                             Field(&AnalogButton::pin, button.pin)));
  }
  return matchers;
}

}  // namespace hs

#endif  // TEST_UTIL_H_
