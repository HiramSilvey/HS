#include "util.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/mock_teensy.h"

namespace hs {

using ::testing::InSequence;
using ::testing::Return;

TEST(UtilTest, GetShortFromEEPROM) {
  MockTeensy teensy;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(1));
    EXPECT_CALL(teensy, EEPROMRead(1)).WillOnce(Return(2));
  }

  EXPECT_EQ(util::GetShortFromEEPROM(teensy, 0), 258);
}

TEST(UtilTest, GetIntFromEEPROM) {
  MockTeensy teensy;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(1));
    EXPECT_CALL(teensy, EEPROMRead(1)).WillOnce(Return(2));
    EXPECT_CALL(teensy, EEPROMRead(2)).WillOnce(Return(3));
    EXPECT_CALL(teensy, EEPROMRead(3)).WillOnce(Return(4));
  }

  EXPECT_EQ(util::GetIntFromEEPROM(teensy, 0), 16909060);
}

}  // namespace hs
