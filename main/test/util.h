#include <gtest/gtest.h>

#include "controller.h"

namespace hs {

std::vector<testing::Matcher<AnalogButton>> AnalogEq(
    const std::vector<AnalogButton>& expected);

}  // namespace hs
