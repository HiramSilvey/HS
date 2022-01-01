#include "ns_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "controller.h"
#include "pins.h"
#include "profile.pb.h"
#include "test/mock_nspad.h"
#include "test/mock_teensy.h"
#include "test/test_util.h"

namespace hs {

using ::testing::_;
using ::testing::AllOf;
using ::testing::AtLeast;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::Return;

auto MappingEq(const NSButtonPinMapping& expected) {
  return AllOf(
      Field(&NSButtonPinMapping::button_id_to_pins, expected.button_id_to_pins),
      Field(&NSButtonPinMapping::mod, expected.mod),
      Field(&NSButtonPinMapping::z_y, ElementsAreArray(AnalogEq(expected.z_y))),
      Field(&NSButtonPinMapping::z_x, ElementsAreArray(AnalogEq(expected.z_x))),
      Field(&NSButtonPinMapping::dpad_up, expected.dpad_up),
      Field(&NSButtonPinMapping::dpad_down, expected.dpad_down),
      Field(&NSButtonPinMapping::dpad_left, expected.dpad_left),
      Field(&NSButtonPinMapping::dpad_right, expected.dpad_right));
}

class NSControllerTest : public ::testing::Test {
 protected:
  NSControllerTest() {
    teensy_ = std::make_unique<MockTeensy>();
    nspad_ = std::make_unique<MockNSPad>();

    EXPECT_CALL(*nspad_, DPadCentered).Times(4).WillRepeatedly(Return(0));
    EXPECT_CALL(*nspad_, DPadRight).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(*nspad_, DPadLeft).Times(2).WillRepeatedly(Return(2));
    EXPECT_CALL(*nspad_, DPadDown).Times(2).WillRepeatedly(Return(3));
    EXPECT_CALL(*nspad_, DPadDownRight).WillOnce(Return(4));
    EXPECT_CALL(*nspad_, DPadDownLeft).WillOnce(Return(5));
    EXPECT_CALL(*nspad_, DPadUp).Times(2).WillRepeatedly(Return(6));
    EXPECT_CALL(*nspad_, DPadUpRight).WillOnce(Return(7));
    EXPECT_CALL(*nspad_, DPadUpLeft).WillOnce(Return(8));

    EXPECT_CALL(*teensy_, DigitalReadLow).Times(AtLeast(1));
    EXPECT_CALL(*teensy_, EEPROMRead).Times(AtLeast(1));
    EXPECT_CALL(*teensy_, Exit);
  }
  std::unique_ptr<MockTeensy> teensy_;
  std::unique_ptr<MockNSPad> nspad_;
};

TEST_F(NSControllerTest, GetButtonPinMapping_StandardDigital) {
  hs_profile_Profile_Layer layer = {
      .thumb_top = DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
      .thumb_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_CIRCLE),
      .thumb_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_TRIANGLE),
      .index_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_SQUARE),
      .index_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L1),
      .middle_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L2),
      .middle_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L3),
      .middle_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R1),
      .ring_top = DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R2),
      .ring_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R3),
      .ring_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_OPTIONS),
      .pinky_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_SHARE),
      .pinky_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_HOME),
      .pinky_bottom =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_CAPTURE)};

  NSButtonPinMapping expected_mapping;
  expected_mapping.button_id_to_pins[1] = {pins::kThumbTop};
  expected_mapping.button_id_to_pins[2] = {pins::kThumbMiddle};
  expected_mapping.button_id_to_pins[3] = {pins::kThumbBottom};
  expected_mapping.button_id_to_pins[0] = {pins::kIndexTop};
  expected_mapping.button_id_to_pins[4] = {pins::kIndexMiddle};
  expected_mapping.button_id_to_pins[6] = {pins::kMiddleTop};
  expected_mapping.button_id_to_pins[10] = {pins::kMiddleMiddle};
  expected_mapping.button_id_to_pins[5] = {pins::kMiddleBottom};
  expected_mapping.button_id_to_pins[7] = {pins::kRingTop};
  expected_mapping.button_id_to_pins[11] = {pins::kRingMiddle};
  expected_mapping.button_id_to_pins[9] = {pins::kRingBottom};
  expected_mapping.button_id_to_pins[8] = {pins::kPinkyTop};
  expected_mapping.button_id_to_pins[12] = {pins::kPinkyMiddle};
  expected_mapping.button_id_to_pins[13] = {pins::kPinkyBottom};

  NSController controller(std::move(teensy_), std::move(nspad_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(NSControllerTest, GetButtonPinMapping_SpecialDigital) {
  hs_profile_Profile_Layer layer = {
      .thumb_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
      .thumb_middle = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_R_STICK_DOWN),
      .thumb_bottom = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_R_STICK_LEFT),
      .left_index_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
      .left_middle_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_UP),
      .left_ring_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_R_STICK_RIGHT),
      .right_index_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
      .right_middle_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
      .right_ring_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_MOD)};

  const int joystick_min = 0;
  const int joystick_max = 255;

  NSButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{joystick_max, pins::kThumbTop},
                          {joystick_min, pins::kThumbMiddle}};
  expected_mapping.z_x = {{joystick_min, pins::kThumbBottom},
                          {joystick_max, pins::kLeftRingExtra}};
  expected_mapping.dpad_up = {pins::kLeftMiddleExtra};
  expected_mapping.dpad_down = {pins::kLeftIndexExtra};
  expected_mapping.dpad_left = {pins::kRightIndexExtra};
  expected_mapping.dpad_right = {pins::kRightMiddleExtra};
  expected_mapping.mod = {pins::kRightRingExtra};

  NSController controller(std::move(teensy_), std::move(nspad_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(NSControllerTest, GetButtonPinMapping_Analog) {
  hs_profile_Profile_Layer layer = {
      .thumb_top = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 100),
      .thumb_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 101)};

  NSButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{100, pins::kThumbTop}};
  expected_mapping.z_x = {{101, pins::kThumbMiddle}};

  NSController controller(std::move(teensy_), std::move(nspad_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(NSControllerTest, GetButtonPinMapping_MultiplePinsOneButton) {
  hs_profile_Profile_Layer layer = {
      .thumb_top = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 100),
      .thumb_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 101),
      .thumb_bottom = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 102),
      .index_top = DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
      .index_middle =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X),
      .middle_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X)};

  NSButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{100, pins::kThumbTop},
                          {101, pins::kThumbMiddle},
                          {102, pins::kThumbBottom}};
  expected_mapping.button_id_to_pins[1] = {pins::kIndexTop, pins::kIndexMiddle,
                                           pins::kMiddleTop};

  NSController controller(std::move(teensy_), std::move(nspad_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}  // namespace hs

TEST_F(NSControllerTest, GetDPadDirection) {
  const uint8_t pin = 1;

  EXPECT_CALL(*teensy_, DigitalReadLow(pin)).WillRepeatedly(Return(true));

  NSController controller(std::move(teensy_), std::move(nspad_));

  NSButtonPinMapping mapping;
  EXPECT_EQ(controller.GetDPadDirection(mapping), 0);

  mapping = {.dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 1);

  mapping = {.dpad_left = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 2);

  mapping = {.dpad_left = {pin}, .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 0);

  mapping = {.dpad_down = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 3);

  mapping = {.dpad_down = {pin}, .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 4);

  mapping = {.dpad_down = {pin}, .dpad_left = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 5);

  mapping = {.dpad_down = {pin}, .dpad_left = {pin}, .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 3);

  mapping = {.dpad_up = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 6);

  mapping = {.dpad_up = {pin}, .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 7);

  mapping = {.dpad_up = {pin}, .dpad_left = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 8);

  mapping = {.dpad_up = {pin}, .dpad_left = {pin}, .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 6);

  mapping = {.dpad_up = {pin}, .dpad_down = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 0);

  mapping = {.dpad_up = {pin}, .dpad_down = {pin}, .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 1);

  mapping = {.dpad_up = {pin}, .dpad_down = {pin}, .dpad_left = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 2);

  mapping = {.dpad_up = {pin},
             .dpad_down = {pin},
             .dpad_left = {pin},
             .dpad_right = {pin}};
  EXPECT_EQ(controller.GetDPadDirection(mapping), 0);
}

TEST_F(NSControllerTest, UpdateButtons) {
  const uint8_t pin = 1;
  const std::vector<int> digital = {pin};
  const std::vector<AnalogButton> analog = {{.value = 0, .pin = pin}};
  NSButtonPinMapping mapping = {.z_y = analog,
                                .z_x = analog,
                                .dpad_up = digital,
                                .dpad_down = digital,
                                .dpad_left = digital,
                                .dpad_right = digital};
  mapping.mod = digital;
  mapping.button_id_to_pins = {
      {0, digital},  {1, digital},  {2, digital},  {3, digital}, {4, digital},
      {5, digital},  {6, digital},  {7, digital},  {8, digital}, {9, digital},
      {10, digital}, {11, digital}, {12, digital}, {13, digital}};

  EXPECT_CALL(*teensy_, DigitalReadLow(1))
      .Times(20)
      .WillRepeatedly(Return(true));

  {
    InSequence seq;
    EXPECT_CALL(*nspad_, SetRightYAxis);
    EXPECT_CALL(*nspad_, SetRightXAxis);
    EXPECT_CALL(*nspad_, Press(_)).Times(14);
    EXPECT_CALL(*nspad_, SetDPad);
  }

  NSController controller(std::move(teensy_), std::move(nspad_));
  controller.UpdateButtons(mapping);
}

}  // namespace hs
