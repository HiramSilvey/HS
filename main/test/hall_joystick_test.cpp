#include "hall_joystick.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "test/mock_teensy.h"

namespace hs {

using ::testing::AllOf;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::Return;

auto CoordinatesEq(const HallJoystick::Coordinates& expected) {
  return AllOf(Field("x", &HallJoystick::Coordinates::x, expected.x),
	       Field("y", &HallJoystick::Coordinates::y, expected.y));
}

class HallJoystickTest : public ::testing::Test {
 protected:
  HallJoystickTest() {
    EXPECT_CALL(teensy_, Pow(2, 8)).WillOnce(Return(256));
    joystick_ = std::make_unique<HallJoystick>(teensy_, /*out_precision=*/8,
					       /*threshold=*/50);
  }

  MockTeensy teensy_;
  std::unique_ptr<HallJoystick> joystick_;
};

TEST_F(HallJoystickTest, GetMin) { EXPECT_EQ(joystick_->get_min(), 0); }
TEST_F(HallJoystickTest, GetMax) { EXPECT_EQ(joystick_->get_max(), 255); }
TEST_F(HallJoystickTest, GetNeutral) {
  EXPECT_EQ(joystick_->get_neutral(), 127);
}

TEST_F(HallJoystickTest, Normalize) {
  EXPECT_EQ(joystick_->Translate(255), 63);
  EXPECT_EQ(joystick_->Translate(511), 127);
  EXPECT_EQ(joystick_->Translate(767), 191);
  EXPECT_EQ(joystick_->Translate(1023), 255);
}

TEST_F(HallJoystickTest, ResolveDigitalCoord_Min) {
  EXPECT_EQ(joystick_->ResolveDigitalCoord(0), 0);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(63), 0);
}

TEST_F(HallJoystickTest, ResolveDigitalCoord_Max) {
  EXPECT_EQ(joystick_->ResolveDigitalCoord(255), 255);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(191), 255);
}

TEST_F(HallJoystickTest, ResolveDigitalCoord_Neutral) {
  EXPECT_EQ(joystick_->ResolveDigitalCoord(127), 127);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(64), 127);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(190), 127);
}

TEST_F(HallJoystickTest, GetCoordinates) {
  {
    InSequence seq;
    EXPECT_CALL(teensy_, AnalogRead).WillOnce(Return(511));   // X
    EXPECT_CALL(teensy_, AnalogRead).WillOnce(Return(1023));  // Y
  }

  // Expect digital resolution.
  HallJoystick::Coordinates expected = {127, 255};
  EXPECT_THAT(joystick_->GetCoordinates(teensy_), CoordinatesEq(expected));
}

}  // namespace hs
