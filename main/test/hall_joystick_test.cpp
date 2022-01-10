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
    // Set x_in_, y_in_ to {-90, 110}
    // Set x_out_, y_out_ to {200, 1200}
    // Set threshold to 50%

    {
      InSequence seq;

      // Set neutral_x (in) to 10
      EXPECT_CALL(teensy_, EEPROMRead(0)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(1)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(2)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(3)).WillOnce(Return(10));
      // Set neutral_y (in) to 10
      EXPECT_CALL(teensy_, EEPROMRead(4)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(5)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(6)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(7)).WillOnce(Return(10));
      // Set range (in) to 100
      EXPECT_CALL(teensy_, EEPROMRead(8)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(9)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(10)).WillOnce(Return(0));
      EXPECT_CALL(teensy_, EEPROMRead(11)).WillOnce(Return(100));
    }

    joystick_ = std::make_unique<HallJoystick>(teensy_, /*min=*/200,
                                               /*max=*/1200, /*threshold=*/50);
  }

  MockTeensy teensy_;
  std::unique_ptr<HallJoystick> joystick_;
};

TEST_F(HallJoystickTest, GetMin) { EXPECT_EQ(joystick_->get_min(), 200); }
TEST_F(HallJoystickTest, GetMax) { EXPECT_EQ(joystick_->get_max(), 1200); }
TEST_F(HallJoystickTest, GetNeutral) {
  EXPECT_EQ(joystick_->get_neutral(), 700);
}

TEST_F(HallJoystickTest, Normalize) {
  const HallJoystick::Bounds in = {.min = -90, .max = 110};

  EXPECT_CALL(teensy_, Constrain(900, 200, 1200));
  joystick_->Normalize(teensy_, 50, in);
}

TEST_F(HallJoystickTest, ResolveDigitalCoord_Min) {
  EXPECT_EQ(joystick_->ResolveDigitalCoord(200), 200);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(450), 200);
}

TEST_F(HallJoystickTest, ResolveDigitalCoord_Max) {
  EXPECT_EQ(joystick_->ResolveDigitalCoord(1200), 1200);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(950), 1200);
}

TEST_F(HallJoystickTest, ResolveDigitalCoord_Neutral) {
  EXPECT_EQ(joystick_->ResolveDigitalCoord(700), 700);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(451), 700);
  EXPECT_EQ(joystick_->ResolveDigitalCoord(949), 700);
}

TEST_F(HallJoystickTest, GetCoordinates) {
  {
    InSequence seq;
    EXPECT_CALL(teensy_, Micros).WillOnce(Return(600));
    EXPECT_CALL(teensy_, UpdateHallData);
    EXPECT_CALL(teensy_, Micros);
    EXPECT_CALL(teensy_, GetHallZ).WillOnce(Return(1));
    EXPECT_CALL(teensy_, GetHallX).WillOnce(Return(0.00005));
    EXPECT_CALL(teensy_, GetHallY).WillOnce(Return(0.00005));
    EXPECT_CALL(teensy_, Constrain(900, 200, 1200))
        .Times(2)
        .WillRepeatedly(Return(900));
  }

  HallJoystick::Coordinates expected = {700, 700};
  EXPECT_THAT(joystick_->GetCoordinates(teensy_), CoordinatesEq(expected));
}

}  // namespace hs
