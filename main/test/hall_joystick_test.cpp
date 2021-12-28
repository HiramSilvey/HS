#include "hall_joystick.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "test/mock_teensy.h"

namespace hs {

using ::testing::Return;

class HallJoystickTest : public ::testing::Test {
 protected:
  HallJoystickTest() {
    // Set x_in_, y_in_ to {-100, 100}
    // Set x_out_, y_out_ to {-200, 200}
    // Set threshold to 50%

    // Set neutral_x (in) to 0
    EXPECT_CALL(teensy_, EEPROMRead(0)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(1)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(2)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(3)).WillOnce(Return(0));
    // Set neutral_y (in) to 0
    EXPECT_CALL(teensy_, EEPROMRead(4)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(5)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(6)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(7)).WillOnce(Return(0));
    // Set range (in) to 100
    EXPECT_CALL(teensy_, EEPROMRead(8)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(9)).WillOnce(Return(0));
    EXPECT_CALL(teensy_, EEPROMRead(10)).WillOnce(Return(6));
    EXPECT_CALL(teensy_, EEPROMRead(11)).WillOnce(Return(4));

    joystick_ = std::make_unique<HallJoystick>(teensy_, /*min=*/-200,
                                               /*max=*/200, /*threshold=*/0.5);
  }

  MockTeensy teensy_;
  std::unique_ptr<HallJoystick> joystick_;
};

TEST_F(HallJoystickTest, GetMin) { EXPECT_EQ(joystick_->get_min(), -200); }

}  // namespace hs
