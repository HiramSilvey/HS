#include "decoder.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_teensy.h"
#include "profile.pb.h"
#include "test_util.h"

namespace hs {

using ::testing::AllOf;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::IsEmpty;
using ::testing::Return;

using Layout = ::hs_profile_Profile_Layout;
using Layer = ::hs_profile_Profile_Layer;
using PlatformConfig = ::hs_profile_Profile_PlatformConfig;
using Platform = ::hs_profile_Profile_Platform;

auto PlatformConfigEq(const PlatformConfig& expected) {
  return AllOf(Field(&PlatformConfig::platform, expected.platform),
               Field(&PlatformConfig::position, expected.position));
}

auto LayerEq(const Layer& expected) {
  return AllOf(
      Field("thumb_top", &Layer::thumb_top, ActionEq(expected.thumb_top)),
      Field("thumb_middle", &Layer::thumb_middle,
            ActionEq(expected.thumb_middle)),
      Field("thumb_bottom", &Layer::thumb_bottom,
            ActionEq(expected.thumb_bottom)),
      Field("index_top", &Layer::index_top, ActionEq(expected.index_top)),
      Field("index_middle", &Layer::index_middle,
            ActionEq(expected.index_middle)),
      Field("middle_top", &Layer::middle_top, ActionEq(expected.middle_top)),
      Field("middle_middle", &Layer::middle_middle,
            ActionEq(expected.middle_middle)),
      Field("middle_bottom", &Layer::middle_bottom,
            ActionEq(expected.middle_bottom)),
      Field("ring_top", &Layer::ring_top, ActionEq(expected.ring_top)),
      Field("ring_middle", &Layer::ring_middle, ActionEq(expected.ring_middle)),
      Field("ring_bottom", &Layer::ring_bottom, ActionEq(expected.ring_bottom)),
      Field("pinky_top", &Layer::pinky_top, ActionEq(expected.pinky_top)),
      Field("pinky_middle", &Layer::pinky_middle,
            ActionEq(expected.pinky_middle)),
      Field("pinky_bottom", &Layer::pinky_bottom,
            ActionEq(expected.pinky_bottom)),
      Field("left_outer", &Layer::left_outer, ActionEq(expected.left_outer)),
      Field("left_inner", &Layer::left_inner, ActionEq(expected.left_inner)));
}

auto BaseLayoutEq(const Layout& expected) {
  return AllOf(Field("joystick_threshold", &Layout::joystick_threshold,
                     expected.joystick_threshold),
               Field("base", &Layout::base, LayerEq(expected.base)),
               Field("has_mod", &Layout::has_mod, expected.has_mod));
}

TEST(DecoderTest, DecodeHeader_SinglePlatform) {
  MockTeensy teensy;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(128));  // 1000000; PC
    EXPECT_CALL(teensy, EEPROMRead(1))
        .WillOnce(Return(16));  // 0001 0000; Position = 1
  }

  EXPECT_THAT(
      decoder::internal::DecodeHeader(teensy, /*addr=*/0),
      ElementsAre(PlatformConfigEq(
          {.platform = hs_profile_Profile_Platform_PC, .position = 1})));
}

TEST(DecoderTest, DecodeHeader_MultiPlatform) {
  MockTeensy teensy;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0))
        .WillOnce(Return(192));  // 11000000; both PC and Switch
    EXPECT_CALL(teensy, EEPROMRead(1))
        .Times(2)
        .WillRepeatedly(
            Return(20));  // 0001 0100; PC position = 1, Switch position = 4
  }

  EXPECT_THAT(
      decoder::internal::DecodeHeader(teensy, /*addr=*/0),
      ElementsAre(
          PlatformConfigEq(
              {.platform = hs_profile_Profile_Platform_PC, .position = 1}),
          PlatformConfigEq({.platform = hs_profile_Profile_Platform_SWITCH,
                            .position = 4})));
}

TEST(DecoderTest, DecodeHeader_UnknownPlatform) {
  MockTeensy teensy;
  EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(0));  // Unknown platform
  EXPECT_THAT(decoder::internal::DecodeHeader(teensy, /*addr=*/0), IsEmpty());
}

TEST(DecoderTest, FetchData_ByteStartsWith) {
  MockTeensy teensy;
  int addr = 0;
  uint8_t curr_byte = 248;  // 11111000
  int unread = 8;

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/5, addr,
                                         curr_byte, unread),
            31);
  EXPECT_EQ(addr, 0);
  EXPECT_EQ(curr_byte, 248);
  EXPECT_EQ(unread, 3);
}

TEST(DecoderTest, FetchData_MiddleOfByte) {
  MockTeensy teensy;
  int addr = 0;
  uint8_t curr_byte = 62;  // 00111110
  int unread = 6;

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/5, addr,
                                         curr_byte, unread),
            31);
  EXPECT_EQ(addr, 0);
  EXPECT_EQ(curr_byte, 62);
  EXPECT_EQ(unread, 1);
}

TEST(DecoderTest, FetchData_ByteEndsWith) {
  MockTeensy teensy;
  int addr = 0;
  uint8_t curr_byte = 31;  // 00011111
  int unread = 5;

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/5, addr,
                                         curr_byte, unread),
            31);
  EXPECT_EQ(addr, 0);
  EXPECT_EQ(curr_byte, 31);
  EXPECT_EQ(unread, 0);
}

TEST(DecoderTest, FetchData_AcrossTwoBytes) {
  MockTeensy teensy;
  int addr = 0;
  uint8_t curr_byte = 3;  // 00000011
  int unread = 2;

  EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(224));  // 11100000

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/5, addr,
                                         curr_byte, unread),
            31);
  EXPECT_EQ(addr, 1);
  EXPECT_EQ(curr_byte, 224);
  EXPECT_EQ(unread, 5);
}

TEST(DecoderTest, FetchData_AcrossThreeBytes) {
  MockTeensy teensy;
  int addr = 0;
  uint8_t curr_byte = 1;  // 00000001
  int unread = 1;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(255));  // 11111111
    EXPECT_CALL(teensy, EEPROMRead(1)).WillOnce(Return(128));  // 10000000
  }

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/10, addr,
                                         curr_byte, unread),
            1023);
  EXPECT_EQ(addr, 2);
  EXPECT_EQ(curr_byte, 128);
  EXPECT_EQ(unread, 7);
}

TEST(DecoderTest, FetchData_WholeByte) {
  MockTeensy teensy;
  int addr = 0;
  uint8_t curr_byte = 255;  // 11111111
  int unread = 8;

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/8, addr,
                                         curr_byte, unread),
            255);
  EXPECT_EQ(addr, 0);
  EXPECT_EQ(curr_byte, 255);
  EXPECT_EQ(unread, 0);
}

TEST(DecoderTest, DecodeLayer) {
  const Layer expected = {
      .thumb_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
      .thumb_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
      .thumb_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_CIRCLE),
      .index_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
      .index_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 1),
      .middle_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R1),
      .middle_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R2),
      .middle_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R3),
      .ring_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_OPTIONS),
      .ring_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 2),
      .ring_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
      .pinky_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
      .pinky_middle = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
      .pinky_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
      .left_outer = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 3),
      .left_inner = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX)};

  MockTeensy teensy;
  int addr = 0;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(0));     // 00000|000
    EXPECT_CALL(teensy, EEPROMRead(1)).WillOnce(Return(68));    // 01|00010|0
    EXPECT_CALL(teensy, EEPROMRead(2)).WillOnce(Return(62));    // 0011|1110
    EXPECT_CALL(teensy, EEPROMRead(3)).WillOnce(Return(0));     // 0|0000000
    EXPECT_CALL(teensy, EEPROMRead(4)).WillOnce(Return(40));    // 001|01000
    EXPECT_CALL(teensy, EEPROMRead(5)).WillOnce(Return(74));    // 01001|010
    EXPECT_CALL(teensy, EEPROMRead(6)).WillOnce(Return(151));   // 10|01011|1
    EXPECT_CALL(teensy, EEPROMRead(7)).WillOnce(Return(208));   // 1101|0000
    EXPECT_CALL(teensy, EEPROMRead(8)).WillOnce(Return(10));    // 000010|10
    EXPECT_CALL(teensy, EEPROMRead(9)).WillOnce(Return(17));    // 000|10001
    EXPECT_CALL(teensy, EEPROMRead(10)).WillOnce(Return(148));  // 10010|100
    EXPECT_CALL(teensy, EEPROMRead(11)).WillOnce(Return(252));  // 11|11110|0
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(1));    // 00000001
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(224));  // 1|11000|00
  }

  EXPECT_THAT(decoder::internal::DecodeLayer(teensy, addr), LayerEq(expected));
  EXPECT_EQ(addr, 14);
}

TEST(DecoderTest, DecodeBody_BaseOnly) {
  const Layout expected = {
      .joystick_threshold = 50,
      // Layer taken from DecodeLayer tests.
      .base = {.thumb_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_NO_OP),
               .thumb_middle =
                   DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
               .thumb_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_CIRCLE),
               .index_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
               .index_middle = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 1),
               .middle_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R1),
               .middle_middle = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R2),
               .middle_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R3),
               .ring_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_OPTIONS),
               .ring_middle = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 2),
               .ring_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
               .pinky_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
               .pinky_middle = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
               .pinky_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
               .left_outer = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 3),
               .left_inner = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX)},
      .has_mod = false};

  MockTeensy teensy;
  int addr = 0;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(15));  // Body length
    EXPECT_CALL(teensy, EEPROMRead(1))
        .WillOnce(Return(50));  // Joystick threshold
    // Layer taken from DecodeLayer tests.
    EXPECT_CALL(teensy, EEPROMRead(2)).WillOnce(Return(0));     // 00000|000
    EXPECT_CALL(teensy, EEPROMRead(3)).WillOnce(Return(68));    // 01|00010|0
    EXPECT_CALL(teensy, EEPROMRead(4)).WillOnce(Return(62));    // 0011|1110
    EXPECT_CALL(teensy, EEPROMRead(5)).WillOnce(Return(0));     // 0|0000000
    EXPECT_CALL(teensy, EEPROMRead(6)).WillOnce(Return(40));    // 001|01000
    EXPECT_CALL(teensy, EEPROMRead(7)).WillOnce(Return(74));    // 01001|010
    EXPECT_CALL(teensy, EEPROMRead(8)).WillOnce(Return(151));   // 10|01011|1
    EXPECT_CALL(teensy, EEPROMRead(9)).WillOnce(Return(208));   // 1101|0000
    EXPECT_CALL(teensy, EEPROMRead(10)).WillOnce(Return(10));   // 000010|10
    EXPECT_CALL(teensy, EEPROMRead(11)).WillOnce(Return(17));   // 000|10001
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(148));  // 10010|100
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(252));  // 11|11110|0
    EXPECT_CALL(teensy, EEPROMRead(14)).WillOnce(Return(1));    // 00000001
    EXPECT_CALL(teensy, EEPROMRead(15)).WillOnce(Return(224));  // 1|11000|00
  }

  EXPECT_THAT(decoder::internal::DecodeBody(teensy, addr),
              BaseLayoutEq(expected));
  EXPECT_EQ(addr, 16);
}

TEST(DecoderTest, DecodeBody_BaseAndMod) {
  // Layer taken from DecodeLayer tests.
  const Layer layer = {
      .thumb_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
      .thumb_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
      .thumb_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_CIRCLE),
      .index_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
      .index_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 1),
      .middle_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R1),
      .middle_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R2),
      .middle_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R3),
      .ring_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_OPTIONS),
      .ring_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 2),
      .ring_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
      .pinky_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
      .pinky_middle = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
      .pinky_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
      .left_outer = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 3),
      .left_inner = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX)};
  const Layout expected = {
      .joystick_threshold = 50, .base = layer, .has_mod = true, .mod = layer};

  MockTeensy teensy;
  int addr = 0;

  {
    InSequence seq;
    EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(29));  // Body length
    EXPECT_CALL(teensy, EEPROMRead(1))
        .WillOnce(Return(50));  // Joystick threshold
    // Base layer; taken from DecodeLayer tests.
    EXPECT_CALL(teensy, EEPROMRead(2)).WillOnce(Return(0));     // 00000|000
    EXPECT_CALL(teensy, EEPROMRead(3)).WillOnce(Return(68));    // 01|00010|0
    EXPECT_CALL(teensy, EEPROMRead(4)).WillOnce(Return(62));    // 0011|1110
    EXPECT_CALL(teensy, EEPROMRead(5)).WillOnce(Return(0));     // 0|0000000
    EXPECT_CALL(teensy, EEPROMRead(6)).WillOnce(Return(40));    // 001|01000
    EXPECT_CALL(teensy, EEPROMRead(7)).WillOnce(Return(74));    // 01001|010
    EXPECT_CALL(teensy, EEPROMRead(8)).WillOnce(Return(151));   // 10|01011|1
    EXPECT_CALL(teensy, EEPROMRead(9)).WillOnce(Return(208));   // 1101|0000
    EXPECT_CALL(teensy, EEPROMRead(10)).WillOnce(Return(10));   // 000010|10
    EXPECT_CALL(teensy, EEPROMRead(11)).WillOnce(Return(17));   // 000|10001
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(148));  // 10010|100
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(252));  // 11|11110|0
    EXPECT_CALL(teensy, EEPROMRead(14)).WillOnce(Return(1));    // 00000001
    EXPECT_CALL(teensy, EEPROMRead(15)).WillOnce(Return(224));  // 1|11000|00
    // Mod layer; taken from DecodeLayer tests.
    EXPECT_CALL(teensy, EEPROMRead(16)).WillOnce(Return(0));    // 00000|000
    EXPECT_CALL(teensy, EEPROMRead(17)).WillOnce(Return(68));   // 01|00010|0
    EXPECT_CALL(teensy, EEPROMRead(18)).WillOnce(Return(62));   // 0011|1110
    EXPECT_CALL(teensy, EEPROMRead(19)).WillOnce(Return(0));    // 0|0000000
    EXPECT_CALL(teensy, EEPROMRead(20)).WillOnce(Return(40));   // 001|01000
    EXPECT_CALL(teensy, EEPROMRead(21)).WillOnce(Return(74));   // 01001|010
    EXPECT_CALL(teensy, EEPROMRead(22)).WillOnce(Return(151));  // 10|01011|1
    EXPECT_CALL(teensy, EEPROMRead(23)).WillOnce(Return(208));  // 1101|0000
    EXPECT_CALL(teensy, EEPROMRead(24)).WillOnce(Return(10));   // 000010|10
    EXPECT_CALL(teensy, EEPROMRead(25)).WillOnce(Return(17));   // 000|10001
    EXPECT_CALL(teensy, EEPROMRead(26)).WillOnce(Return(148));  // 10010|100
    EXPECT_CALL(teensy, EEPROMRead(27)).WillOnce(Return(252));  // 11|11110|0
    EXPECT_CALL(teensy, EEPROMRead(28)).WillOnce(Return(1));    // 00000001
    EXPECT_CALL(teensy, EEPROMRead(29)).WillOnce(Return(224));  // 1|11000|00
  }

  EXPECT_THAT(decoder::internal::DecodeBody(teensy, addr),
              AllOf(BaseLayoutEq(expected),
                    Field("mod", &Layout::mod, LayerEq(expected.mod))));
  EXPECT_EQ(addr, 30);
}

TEST(DecoderTest, Decode_FirstProfile) {
  const Layout expected = {
      .joystick_threshold = 50,
      // Layer taken from DecodeLayer tests.
      .base = {.thumb_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_NO_OP),
               .thumb_middle =
                   DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
               .thumb_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_CIRCLE),
               .index_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
               .index_middle = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 1),
               .middle_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R1),
               .middle_middle = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R2),
               .middle_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R3),
               .ring_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_OPTIONS),
               .ring_middle = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 2),
               .ring_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
               .pinky_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
               .pinky_middle = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
               .pinky_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
               .left_outer = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 3),
               .left_inner = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX)},
      .has_mod = false};

  MockTeensy teensy;

  {
    InSequence seq;

    // Encoded length = 18.
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(0));
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(18));
    // Header; taken from DecodeHeader tests.
    EXPECT_CALL(teensy, EEPROMRead(14)).WillOnce(Return(128));  // 1000000; PC
    EXPECT_CALL(teensy, EEPROMRead(15))
        .WillOnce(Return(16));  // 0001 0000; Position = 1
    // Body; taken from DecodeBody tests.
    EXPECT_CALL(teensy, EEPROMRead(16)).WillOnce(Return(15));  // Body length
    EXPECT_CALL(teensy, EEPROMRead(17))
        .WillOnce(Return(50));  // Joystick threshold
    // Layer; taken from DecodeLayer tests.
    EXPECT_CALL(teensy, EEPROMRead(18)).WillOnce(Return(0));    // 00000|000
    EXPECT_CALL(teensy, EEPROMRead(19)).WillOnce(Return(68));   // 01|00010|0
    EXPECT_CALL(teensy, EEPROMRead(20)).WillOnce(Return(62));   // 0011|1110
    EXPECT_CALL(teensy, EEPROMRead(21)).WillOnce(Return(0));    // 0|0000000
    EXPECT_CALL(teensy, EEPROMRead(22)).WillOnce(Return(40));   // 001|01000
    EXPECT_CALL(teensy, EEPROMRead(23)).WillOnce(Return(74));   // 01001|010
    EXPECT_CALL(teensy, EEPROMRead(24)).WillOnce(Return(151));  // 10|01011|1
    EXPECT_CALL(teensy, EEPROMRead(25)).WillOnce(Return(208));  // 1101|0000
    EXPECT_CALL(teensy, EEPROMRead(26)).WillOnce(Return(10));   // 000010|10
    EXPECT_CALL(teensy, EEPROMRead(27)).WillOnce(Return(17));   // 000|10001
    EXPECT_CALL(teensy, EEPROMRead(28)).WillOnce(Return(148));  // 10010|100
    EXPECT_CALL(teensy, EEPROMRead(29)).WillOnce(Return(252));  // 11|11110|0
    EXPECT_CALL(teensy, EEPROMRead(30)).WillOnce(Return(1));    // 00000001
    EXPECT_CALL(teensy, EEPROMRead(31)).WillOnce(Return(224));  // 1|11000|00
  }

  EXPECT_THAT(
      decoder::Decode(teensy, hs_profile_Profile_Platform_PC, /*position=*/1),
      BaseLayoutEq(expected));
}

TEST(DecoderTest, Decode_SecondProfile) {
  const Layout expected = {
      .joystick_threshold = 50,
      // Layer taken from DecodeLayer tests.
      .base = {.thumb_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_NO_OP),
               .thumb_middle =
                   DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
               .thumb_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_CIRCLE),
               .index_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
               .index_middle = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 1),
               .middle_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R1),
               .middle_middle = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R2),
               .middle_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R3),
               .ring_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_OPTIONS),
               .ring_middle = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 2),
               .ring_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
               .pinky_top = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
               .pinky_middle = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
               .pinky_bottom = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
               .left_outer = AnalogLayerAction(
                   hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 3),
               .left_inner = DigitalLayerAction(
                   hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX)},
      .has_mod = false};

  MockTeensy teensy;

  {
    InSequence seq;

    // Encoded length = 36.
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(0));
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(36));
    // First profile header.
    EXPECT_CALL(teensy, EEPROMRead(14))
        .WillOnce(Return(64));  // 0100000; Switch
    EXPECT_CALL(teensy, EEPROMRead(15))
        .WillOnce(Return(64));  // 0100 0000; Position = 4
    // First profile body.
    EXPECT_CALL(teensy, EEPROMRead(16)).WillOnce(Return(15));  // Body length

    // Second profile header; taken from DecodeHeader tests.
    EXPECT_CALL(teensy, EEPROMRead(32)).WillOnce(Return(128));  // 1000000; PC
    EXPECT_CALL(teensy, EEPROMRead(33))
        .WillOnce(Return(16));  // 0001 0000; Position = 1
    // Second profile body; taken from DecodeBody tests.
    EXPECT_CALL(teensy, EEPROMRead(34)).WillOnce(Return(15));  // Body length
    EXPECT_CALL(teensy, EEPROMRead(35))
        .WillOnce(Return(50));  // Joystick threshold
    // Second profile layer; taken from DecodeLayer tests.
    EXPECT_CALL(teensy, EEPROMRead(36)).WillOnce(Return(0));    // 00000|000
    EXPECT_CALL(teensy, EEPROMRead(37)).WillOnce(Return(68));   // 01|00010|0
    EXPECT_CALL(teensy, EEPROMRead(38)).WillOnce(Return(62));   // 0011|1110
    EXPECT_CALL(teensy, EEPROMRead(39)).WillOnce(Return(0));    // 0|0000000
    EXPECT_CALL(teensy, EEPROMRead(40)).WillOnce(Return(40));   // 001|01000
    EXPECT_CALL(teensy, EEPROMRead(41)).WillOnce(Return(74));   // 01001|010
    EXPECT_CALL(teensy, EEPROMRead(42)).WillOnce(Return(151));  // 10|01011|1
    EXPECT_CALL(teensy, EEPROMRead(43)).WillOnce(Return(208));  // 1101|0000
    EXPECT_CALL(teensy, EEPROMRead(44)).WillOnce(Return(10));   // 000010|10
    EXPECT_CALL(teensy, EEPROMRead(45)).WillOnce(Return(17));   // 000|10001
    EXPECT_CALL(teensy, EEPROMRead(46)).WillOnce(Return(148));  // 10010|100
    EXPECT_CALL(teensy, EEPROMRead(47)).WillOnce(Return(252));  // 11|11110|0
    EXPECT_CALL(teensy, EEPROMRead(48)).WillOnce(Return(1));    // 00000001
    EXPECT_CALL(teensy, EEPROMRead(49)).WillOnce(Return(224));  // 1|11000|00
  }

  EXPECT_THAT(
      decoder::Decode(teensy, hs_profile_Profile_Platform_PC, /*position=*/1),
      BaseLayoutEq(expected));
}

// This test data comes from a real profile data dump.
TEST(DecoderTest, Decode_BaseAndMod) {
  const Layout expected = {
      .joystick_threshold = 0,
      .base =
          {.thumb_top =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R2),
           .thumb_middle =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L2),
           .thumb_bottom =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L3),
           .index_top = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
           .index_middle =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
           .middle_top = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_SQUARE),
           .middle_middle = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_CIRCLE),
           .middle_bottom =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R3),
           .ring_top = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_R_STICK_LEFT),
           .ring_middle =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L1),
           .ring_bottom = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_R_STICK_DOWN),
           .pinky_top = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_R_STICK_RIGHT),
           .pinky_middle =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R1),
           .pinky_bottom = DigitalLayerAction(
               hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
           .left_outer =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_SHARE),
           .left_inner =
               DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_MOD)},
      .has_mod = true,
      .mod = {
          .thumb_top = DigitalLayerAction(
              hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
          .thumb_middle =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .thumb_bottom =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .index_top = DigitalLayerAction(
              hs_profile_Profile_Layer_DigitalAction_OPTIONS),
          .index_middle = DigitalLayerAction(
              hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
          .middle_top =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .middle_middle = DigitalLayerAction(
              hs_profile_Profile_Layer_DigitalAction_D_PAD_UP),
          .middle_bottom =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .ring_top =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .ring_middle = DigitalLayerAction(
              hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
          .ring_bottom =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .pinky_top =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .pinky_middle =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .pinky_bottom =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .left_outer =
              DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_NO_OP),
          .left_inner = DigitalLayerAction(
              hs_profile_Profile_Layer_DigitalAction_NO_OP)}};

  MockTeensy teensy;

  {
    InSequence seq;

    // Encoded length = 24.
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(0));
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(24));
    // Header.
    EXPECT_CALL(teensy, EEPROMRead(14)).WillOnce(Return(128));  // 1000000; PC
    EXPECT_CALL(teensy, EEPROMRead(15))
        .WillOnce(Return(0));  // 0000 0000; Position = 0
    // Body.
    EXPECT_CALL(teensy, EEPROMRead(16)).WillOnce(Return(21));  // Body length
    EXPECT_CALL(teensy, EEPROMRead(17))
        .WillOnce(Return(0));  // Joystick threshold
    // Layer.
    EXPECT_CALL(teensy, EEPROMRead(18)).WillOnce(Return(73));   // 01001|001
    EXPECT_CALL(teensy, EEPROMRead(19)).WillOnce(Return(142));  // 10|00111|0
    EXPECT_CALL(teensy, EEPROMRead(20)).WillOnce(Return(48));   // 0011|0000
    EXPECT_CALL(teensy, EEPROMRead(21)).WillOnce(Return(144));  // 1|00100|00
    EXPECT_CALL(teensy, EEPROMRead(22)).WillOnce(Return(74));   // 010|01010
    EXPECT_CALL(teensy, EEPROMRead(23)).WillOnce(Return(169));  // 10101|001
    EXPECT_CALL(teensy, EEPROMRead(24)).WillOnce(Return(105));  // 01|10100|1
    EXPECT_CALL(teensy, EEPROMRead(25)).WillOnce(Return(100));  // 0110|0100
    EXPECT_CALL(teensy, EEPROMRead(26)).WillOnce(Return(77));   // 0|10011|01
    EXPECT_CALL(teensy, EEPROMRead(27)).WillOnce(Return(155));  // 100|11011
    EXPECT_CALL(teensy, EEPROMRead(28)).WillOnce(Return(128));  // 10000|000
    EXPECT_CALL(teensy, EEPROMRead(29)).WillOnce(Return(0));    // 00|00000|0
    EXPECT_CALL(teensy, EEPROMRead(30)).WillOnce(Return(184));  // 1011|1000
    EXPECT_CALL(teensy, EEPROMRead(31)).WillOnce(Return(129));  // 1|00000|01
    EXPECT_CALL(teensy, EEPROMRead(32)).WillOnce(Return(224));  // 111|00000
    EXPECT_CALL(teensy, EEPROMRead(33)).WillOnce(Return(4));    // 00000|100
    EXPECT_CALL(teensy, EEPROMRead(34)).WillOnce(Return(128));  // 10|00000|0
    EXPECT_CALL(teensy, EEPROMRead(35)).WillOnce(Return(0));    // 0000|0000
    EXPECT_CALL(teensy, EEPROMRead(36)).WillOnce(Return(0));    // 0|00000|00
    EXPECT_CALL(teensy, EEPROMRead(37)).WillOnce(Return(0));    // 000|00000
  }

  EXPECT_THAT(decoder::Decode(teensy, hs_profile_Profile_Platform_PC,
                              /*position=*/0),
              AllOf(BaseLayoutEq(expected),
                    Field("mod", &Layout::mod, LayerEq(expected.mod))));
}

TEST(DecoderTest, Decode_ProfileNotFound) {
  MockTeensy teensy;

  {
    InSequence seq;

    // Encoded length = 18.
    EXPECT_CALL(teensy, EEPROMRead(12)).WillOnce(Return(0));
    EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(18));
    // Header; taken from DecodeHeader tests.
    EXPECT_CALL(teensy, EEPROMRead(14)).WillOnce(Return(128));  // 1000000; PC
    EXPECT_CALL(teensy, EEPROMRead(15))
        .WillOnce(Return(16));  // 0001 0000; Position = 1
    // Body; taken from DecodeBody tests.
    EXPECT_CALL(teensy, EEPROMRead(16)).WillOnce(Return(15));  // Body length

    EXPECT_CALL(teensy, Exit(1));

    // Ignore additional calls that won't be reached with an implemented 'Exit'
    // function.
    EXPECT_CALL(teensy, EEPROMRead).WillRepeatedly(Return(0));
  }

  decoder::Decode(teensy, hs_profile_Profile_Platform_SWITCH, /*position=*/1);
}

}  // namespace hs
