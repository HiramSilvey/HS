#include "pc_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "mock_teensy.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

class PCControllerTest : public ::testing::Test {
protected:
  PCControllerTest() {
    teensy_ = std::make_unique<MockTeensy>();

    EXPECT_CALL(*teensy_, DigitalReadLow)
      .Times(AtLeast(1));
    EXPECT_CALL(*teensy_, EEPROMRead)
      .Times(AtLeast(1));
    EXPECT_CALL(*teensy_, Exit);
    EXPECT_CALL(*teensy_, JoystickUseManualSend);
  }
  std::unique_ptr<MockTeensy> teensy_;
};

TEST_F(PCControllerTest, GetDPadAngle) {
  const uint8_t pin = 1;

  EXPECT_CALL(*teensy_, DigitalReadLow(pin))
    .WillRepeatedly(Return(true));

  PCController controller(std::move(teensy_));

  PCButtonPinMapping mapping;
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 90);

  mapping = {
    .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 270);

  mapping = {
    .hat_left = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {
    .hat_down = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 180);

  mapping = {
    .hat_down = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 135);

  mapping = {
    .hat_down = {pin},
    .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 225);

  mapping = {
    .hat_down = {pin},
    .hat_left = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 180);

  mapping = {
      .hat_up = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 0);

  mapping = {
    .hat_up = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 45);

  mapping = {
    .hat_up = {pin},
    .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 315);

  mapping = {
    .hat_up = {pin},
    .hat_left = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 0);

  mapping = {
    .hat_up = {pin},
    .hat_down = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {
    .hat_up = {pin},
    .hat_down = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 90);

  mapping = {
    .hat_up = {pin},
    .hat_down = {pin},
    .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 270);

  mapping = {
    .hat_up = {pin},
    .hat_down = {pin},
    .hat_left = {pin},
    .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);
}
