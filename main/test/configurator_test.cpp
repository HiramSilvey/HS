#include "configurator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/mock_teensy.h"

namespace hs {

using ::testing::Return;

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
  for (int i = 0; i < 3; i++) {
    EXPECT_CALL(teensy, EEPROMUpdate(i, 1));
  }
  EXPECT_CALL(teensy, SerialWrite(0));

  configurator::StoreProfiles(teensy);
}

}  // namespace hs
