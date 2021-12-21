#include "pc_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "mock_mcu.h"

using ::testing::Return;
using ::testing::_;

TEST(PCControllerTest, GetDPadAngleUp) {
  auto mcu = std::make_unique<MockMCU>();

  EXPECT_CALL(mcu, DigitalReadLow(_))
    .WillOnce(Return(true));

  PCController controller(std::move(mcu));

  const PCButtonPinMapping mapping = {
    .hat_up = {1},
  };

  EXPECT_THAT(controller.GetDPadAngle(mapping), Equals(0));
}
