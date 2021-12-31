#ifndef TEST_UTIL_H_
#define TEST_UTIL_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "controller.h"

namespace hs {

using ::testing::AllOf;
using ::testing::Field;
using ::testing::Matcher;
using Action = ::hs_profile_Profile_Layer_Action;

MATCHER_P(ActionTypeEq, expected, "") {
  return arg.action_type.digital == expected.digital &&
         arg.action_type.analog.id == expected.analog.id &&
         arg.action_type.analog.value == expected.analog.value;
}

auto ActionEq(const Action& expected) {
  return AllOf(Field(&Action::which_action_type, expected.which_action_type),
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
