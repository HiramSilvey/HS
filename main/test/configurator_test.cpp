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

    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_x)));
    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_y)));
    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_range)));
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
    uint8_t expected_x[4] = {0, 30, 132, 128};
    EXPECT_CALL(teensy, GetHallY).WillOnce(Return(0.3));
    uint8_t expected_y[4] = {0, 45, 198, 192};

    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_x)));
    EXPECT_CALL(teensy, SerialWrite(_, 4))
        .With(Args<0, 1>(ElementsAreArray(expected_y)));
  }

  configurator::internal::FetchJoystickCoords(teensy);
}

}  // namespace hs
