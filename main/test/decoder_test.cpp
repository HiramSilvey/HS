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
      Field("left_index_extra", &Layer::left_index_extra,
            ActionEq(expected.left_index_extra)),
      Field("left_middle_extra", &Layer::left_middle_extra,
            ActionEq(expected.left_middle_extra)),
      Field("left_ring_extra", &Layer::left_ring_extra,
            ActionEq(expected.left_ring_extra)),
      Field("right_index_extra", &Layer::right_index_extra,
            ActionEq(expected.right_index_extra)),
      Field("right_middle_extra", &Layer::right_middle_extra,
            ActionEq(expected.right_middle_extra)),
      Field("right_ring_extra", &Layer::right_ring_extra,
            ActionEq(expected.right_ring_extra)));
}

auto LayoutEq(const Layout& expected) {
  return AllOf(Field("joystick_threshold", &Layout::joystick_threshold,
                     expected.joystick_threshold),
               Field("has_base", &Layout::has_base, expected.has_base),
               Field("base", &Layout::base, LayerEq(expected.base)),
               Field("has_mod", &Layout::has_mod, expected.has_mod),
               Field("mod", &Layout::mod, LayerEq(expected.mod)));
}

TEST(DecoderTest, DecodeHeader_SinglePlatform) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(128));  // 1000000; PC
  EXPECT_CALL(teensy, EEPROMRead(1))
      .WillOnce(Return(16));  // 0001 0000; Position = 1

  EXPECT_THAT(
      decoder::internal::DecodeHeader(teensy, /*addr=*/0),
      ElementsAre(PlatformConfigEq(
          {.platform = hs_profile_Profile_Platform_PC, .position = 1})));
}

TEST(DecoderTest, DecodeHeader_MultiPlatform) {
  MockTeensy teensy;

  EXPECT_CALL(teensy, EEPROMRead(0))
      .WillOnce(Return(192));  // 11000000; both PC and Switch
  EXPECT_CALL(teensy, EEPROMRead(1))
      .Times(2)
      .WillRepeatedly(
          Return(20));  // 0001 0100; PC position = 1, Switch position = 4

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

  EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(123));

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/5, addr,
                                         curr_byte, unread),
            31);
  EXPECT_EQ(addr, 1);
  EXPECT_EQ(curr_byte, 123);
  EXPECT_EQ(unread, 8);
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

  EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(255));  // 11111111
  EXPECT_CALL(teensy, EEPROMRead(1)).WillOnce(Return(128));  // 10000000

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

  EXPECT_CALL(teensy, EEPROMRead(0)).WillOnce(Return(123));

  EXPECT_EQ(decoder::internal::FetchData(teensy, /*remaining=*/8, addr,
                                         curr_byte, unread),
            255);
  EXPECT_EQ(addr, 1);
  EXPECT_EQ(curr_byte, 123);
  EXPECT_EQ(unread, 8);
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
      .left_index_extra = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 3),
      .left_middle_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX),
      .left_ring_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MIN),
      .right_index_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MAX),
      .right_middle_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_MOD),
      .right_ring_extra = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_RIGHT, 4)};

  MockTeensy teensy;
  int addr = 0;

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
  EXPECT_CALL(teensy, EEPROMRead(13)).WillOnce(Return(227));  // 1|11000|11
  EXPECT_CALL(teensy, EEPROMRead(14)).WillOnce(Return(58));   // 001|11010
  EXPECT_CALL(teensy, EEPROMRead(15)).WillOnce(Return(223));  // 11011|111
  EXPECT_CALL(teensy, EEPROMRead(16)).WillOnce(Return(192));  // 11|000000
  EXPECT_CALL(teensy, EEPROMRead(17)).WillOnce(Return(64));   // 0100|0000

  EXPECT_THAT(decoder::internal::DecodeLayer(teensy, addr), LayerEq(expected));
}

}  // namespace hs
