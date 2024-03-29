#include "configurator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/mock_teensy.h"

namespace hs {

using ::testing::_;
using ::testing::Args;
using ::testing::ElementsAreArray;
using ::testing::InSequence;
using ::testing::Return;

TEST(ConfiguratorTest, FetchStoredBounds) {
  MockTeensy teensy;

  {
    InSequence seq;

    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(1));
    EXPECT_CALL(teensy, EEPROMRead(1)).WillOnce(Return(1));
    EXPECT_CALL(teensy, EEPROMRead(2)).WillOnce(Return(1));
    EXPECT_CALL(teensy, EEPROMRead(3)).WillOnce(Return(1));
    uint8_t expected_x[4] = {1, 1, 1, 1};

    EXPECT_CALL(teensy, EEPROMRead(4)).WillOnce(Return(2));
    EXPECT_CALL(teensy, EEPROMRead(5)).WillOnce(Return(2));
    EXPECT_CALL(teensy, EEPROMRead(6)).WillOnce(Return(2));
    EXPECT_CALL(teensy, EEPROMRead(7)).WillOnce(Return(2));
    uint8_t expected_y[4] = {2, 2, 2, 2};

    EXPECT_CALL(teensy, EEPROMRead(8)).WillOnce(Return(3));
    EXPECT_CALL(teensy, EEPROMRead(9)).WillOnce(Return(3));
    EXPECT_CALL(teensy, EEPROMRead(10)).WillOnce(Return(3));
    EXPECT_CALL(teensy, EEPROMRead(11)).WillOnce(Return(3));
    uint8_t expected_range[4] = {3, 3, 3, 3};

    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(4));
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(4));
    uint8_t expected_angle_ticks[2] = {4, 4};

    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_x)));
    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_y)));
    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_range)));
    EXPECT_CALL(teensy, SerialWrite(_, 2))
        .With(Args<0, 1>(ElementsAreArray(expected_angle_ticks)));
  }

  configurator::internal::FetchStoredBounds(teensy);
}

TEST(ConfiguratorTest, FetchJoystickCoords) {
  MockTeensy teensy;

  {
    InSequence seq;
    EXPECT_CALL(teensy, UpdateHallData);
    EXPECT_CALL(teensy, GetHallZ).WillOnce(Return(0.1));
    EXPECT_CALL(teensy, GetHallX).WillOnce(Return(0.2));
    uint8_t expected_x[4] = {0, 30, 132, 128};  // 2000000
    EXPECT_CALL(teensy, GetHallY).WillOnce(Return(0.3));
    uint8_t expected_y[4] = {0, 45, 198, 192};  // 3000000

    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_x)));
    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_y)));
  }

  configurator::internal::FetchJoystickCoords(teensy);
}

TEST(ConfiguratorTest, CalibrateJoystick_RangeX) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, Millis)
      .Times(4)
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(15000));
  EXPECT_CALL(teensy, UpdateHallData).Times(2);
  EXPECT_CALL(teensy, GetHallZ).Times(2).WillRepeatedly(Return(0.1));
  EXPECT_CALL(teensy, GetHallX)
      .Times(2)
      .WillOnce(Return(0.3))
      .WillOnce(Return(-0.4));
  EXPECT_CALL(teensy, GetHallY)
      .Times(2)
      .WillOnce(Return(0.2))
      .WillOnce(Return(-0.3));
  uint8_t center[4] = {255, 248, 94, 224};  // -500000
  uint8_t range[4] = {0, 53, 103, 224};     // 3500000

  EXPECT_CALL(teensy, SerialWrite(_, 4))
      .With(Args<0, 1>(ElementsAreArray(center)))
      .Times(2);
  EXPECT_CALL(teensy, SerialWrite(_, 4))
      .With(Args<0, 1>(ElementsAreArray(range)));

  configurator::internal::CalibrateJoystick(teensy);
}

TEST(ConfiguratorTest, CalibrateJoystick_RangeY) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, Millis)
      .Times(4)
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(15000));
  EXPECT_CALL(teensy, UpdateHallData).Times(2);
  EXPECT_CALL(teensy, GetHallZ).Times(2).WillRepeatedly(Return(0.1));
  EXPECT_CALL(teensy, GetHallX)
      .Times(2)
      .WillOnce(Return(0.2))
      .WillOnce(Return(-0.3));
  EXPECT_CALL(teensy, GetHallY)
      .Times(2)
      .WillOnce(Return(0.3))
      .WillOnce(Return(-0.4));
  uint8_t center[4] = {255, 248, 94, 224};  // -500000
  uint8_t range[4] = {0, 53, 103, 224};     // 3500000

  EXPECT_CALL(teensy, SerialWrite(_, 4))
      .With(Args<0, 1>(ElementsAreArray(center)))
      .Times(2);
  EXPECT_CALL(teensy, SerialWrite(_, 4))
      .With(Args<0, 1>(ElementsAreArray(range)));

  configurator::internal::CalibrateJoystick(teensy);
}

TEST(ConfiguratorTest, SaveCalibration) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, SerialAvailable).Times(14).WillRepeatedly(Return(true));
  EXPECT_CALL(teensy, SerialRead).Times(14).WillRepeatedly(Return(1));
  for (int i = 0; i < 14; i++) {
    EXPECT_CALL(teensy, EEPROMUpdate(i, 1));
  }
  EXPECT_CALL(teensy, SerialWrite(0));

  configurator::internal::SaveCalibration(teensy);
}

TEST(ConfiguratorTest, SaveCalibration_SerialUnavailable) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, SerialAvailable)
      .Times(15)
      .WillOnce(Return(false))  // Expect it to retry upon seeing false.
      .WillRepeatedly(Return(true));
  EXPECT_CALL(teensy, SerialRead).Times(14).WillRepeatedly(Return(1));
  for (int i = 0; i < 14; i++) {
    EXPECT_CALL(teensy, EEPROMUpdate(i, 1));
  }
  EXPECT_CALL(teensy, SerialWrite(0));

  configurator::internal::SaveCalibration(teensy);
}

TEST(ConfiguratorTest, StoreProfiles) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, SerialAvailable)
      .WillOnce(Return(1))  // Expect it to retry upon seeing less than 2.
      .WillOnce(Return(2))
      .WillOnce(Return(false))  // Expect it to retry upon seeing false.
      .WillOnce(Return(true))
      .WillOnce(Return(true))
      .WillOnce(Return(true));
  EXPECT_CALL(teensy, SerialRead)
      .WillOnce(Return(0))
      .WillOnce(Return(3))
      .WillOnce(Return(1))
      .WillOnce(Return(1))
      .WillOnce(Return(1));
  for (int i = 14; i < 17; i++) {
    EXPECT_CALL(teensy, EEPROMUpdate(i, 1));
  }
  EXPECT_CALL(teensy, SerialWrite(0));

  configurator::internal::StoreProfiles(teensy);
}

}  // namespace hs
