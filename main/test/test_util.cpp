#include "test_util.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "controller.h"

namespace hs {

using ::testing::AllOf;
using ::testing::Field;
using ::testing::Matcher;

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
