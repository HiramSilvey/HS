#include "controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "mock_teensy.h"
#include "profile.pb.h"

namespace hs {

using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;

TEST(ControllerTest, FetchProfile) {
  const auto teensy = std::make_unique<MockTeensy>();

  EXPECT_CALL(*teensy, DigitalReadLow).Times(12);
  EXPECT_CALL(*teensy, EEPROMRead).Times(AtLeast(1));
  EXPECT_CALL(*teensy, Exit);

  FetchProfile(*teensy, hs_profile_Profile_Platform_PC);
}

TEST(ControllerTest, ResolveSOCD_Min) {
  auto mock_teensy = std::make_unique<MockTeensy>();
  const std::vector<AnalogButton> buttons = {{.value = 100, .pin = 1},
                                             {.value = -75, .pin = 2}};
  const int joystick_neutral = 0;

  {
    InSequence seq;
    EXPECT_CALL(*mock_teensy, DigitalReadLow(1)).WillOnce(Return(false));
    EXPECT_CALL(*mock_teensy, DigitalReadLow(2)).WillOnce(Return(true));
  }

  std::unique_ptr<Teensy> teensy = std::move(mock_teensy);
  EXPECT_EQ(ResolveSOCD(*teensy, buttons, joystick_neutral), -75);
}

TEST(ControllerTest, ResolveSOCD_Max) {
  auto teensy = std::make_unique<MockTeensy>();
  const std::vector<AnalogButton> buttons = {{.value = 100, .pin = 1},
                                             {.value = -75, .pin = 2}};
  const int joystick_neutral = 0;

  {
    InSequence seq;
    EXPECT_CALL(*teensy, DigitalReadLow(1)).WillOnce(Return(true));
    EXPECT_CALL(*teensy, DigitalReadLow(2)).WillOnce(Return(false));
  }

  EXPECT_EQ(ResolveSOCD(*teensy, buttons, joystick_neutral), 100);
}

TEST(ControllerTest, ResolveSOCD_Cancel) {
  auto teensy = std::make_unique<MockTeensy>();
  const std::vector<AnalogButton> buttons = {{.value = 100, .pin = 1},
                                             {.value = -75, .pin = 2}};
  const int joystick_neutral = 0;

  {
    InSequence seq;
    EXPECT_CALL(*teensy, DigitalReadLow(1)).WillOnce(Return(true));
    EXPECT_CALL(*teensy, DigitalReadLow(2)).WillOnce(Return(true));
  }

  EXPECT_EQ(ResolveSOCD(*teensy, buttons, joystick_neutral), 0);
}

TEST(ControllerTest, ResolveSOCD_LargerMax) {
  auto teensy = std::make_unique<MockTeensy>();
  const std::vector<AnalogButton> buttons = {{.value = 100, .pin = 1},
                                             {.value = 125, .pin = 2},
                                             {.value = -75, .pin = 3}};
  const int joystick_neutral = 0;

  {
    InSequence seq;
    EXPECT_CALL(*teensy, DigitalReadLow(1)).WillOnce(Return(true));
    EXPECT_CALL(*teensy, DigitalReadLow(2)).WillOnce(Return(true));
    EXPECT_CALL(*teensy, DigitalReadLow(3)).WillOnce(Return(false));
  }

  EXPECT_EQ(ResolveSOCD(*teensy, buttons, joystick_neutral), 125);
}

TEST(ControllerTest, ResolveSOCD_SmallerMin) {
  auto teensy = std::make_unique<MockTeensy>();
  const std::vector<AnalogButton> buttons = {{.value = 100, .pin = 1},
                                             {.value = -75, .pin = 2},
                                             {.value = -110, .pin = 3}};
  const int joystick_neutral = 0;

  {
    InSequence seq;
    EXPECT_CALL(*teensy, DigitalReadLow(1)).WillOnce(Return(false));
    EXPECT_CALL(*teensy, DigitalReadLow(2)).WillOnce(Return(true));
    EXPECT_CALL(*teensy, DigitalReadLow(3)).WillOnce(Return(true));
  }

  EXPECT_EQ(ResolveSOCD(*teensy, buttons, joystick_neutral), -110);
}

}  // namespace hs
