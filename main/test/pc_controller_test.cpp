#include "pc_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "mock_teensy.h"

using ::testing::_;
using ::testing::Return;

TEST(PCControllerTest, GetDPadAngleUp) {
  auto teensy = std::make_unique<MockTeensy>();

  EXPECT_CALL(teensy, DigitalReadLow(_)).WillOnce(Return(true));

  PCController controller(std::move(teensy));

  const PCButtonPinMapping mapping = {
      .hat_up = {1},
  };

  EXPECT_THAT(controller.GetDPadAngle(mapping), Equals(0));
}
