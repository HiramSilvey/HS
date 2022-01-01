#include "pc_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "controller.h"
#include "pins.h"
#include "profile.pb.h"
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

auto MappingEq(const PCButtonPinMapping& expected) {
  return AllOf(
      Field(&PCButtonPinMapping::button_id_to_pins, expected.button_id_to_pins),
      Field(&PCButtonPinMapping::mod, expected.mod),
      Field(&PCButtonPinMapping::z_y, ElementsAreArray(AnalogEq(expected.z_y))),
      Field(&PCButtonPinMapping::z_x, ElementsAreArray(AnalogEq(expected.z_x))),
      Field(&PCButtonPinMapping::slider_left,
            ElementsAreArray(AnalogEq(expected.slider_left))),
      Field(&PCButtonPinMapping::slider_right,
            ElementsAreArray(AnalogEq(expected.slider_right))),
      Field(&PCButtonPinMapping::hat_up, expected.hat_up),
      Field(&PCButtonPinMapping::hat_down, expected.hat_down),
      Field(&PCButtonPinMapping::hat_left, expected.hat_left),
      Field(&PCButtonPinMapping::hat_right, expected.hat_right));
}

class PCControllerTest : public ::testing::Test {
 protected:
  PCControllerTest() {
    teensy_ = std::make_unique<MockTeensy>();

    EXPECT_CALL(*teensy_, DigitalReadLow).Times(AtLeast(1));
    EXPECT_CALL(*teensy_, EEPROMRead).Times(AtLeast(1));
    EXPECT_CALL(*teensy_, Exit);
    EXPECT_CALL(*teensy_, JoystickUseManualSend);
  }
  std::unique_ptr<MockTeensy> teensy_;
};

TEST_F(PCControllerTest, GetButtonPinMapping_StandardDigital) {
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
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_SHARE)};

  PCButtonPinMapping expected_mapping;
  expected_mapping.button_id_to_pins[2] = {pins::kThumbTop};
  expected_mapping.button_id_to_pins[3] = {pins::kThumbMiddle};
  expected_mapping.button_id_to_pins[4] = {pins::kThumbBottom};
  expected_mapping.button_id_to_pins[1] = {pins::kIndexTop};
  expected_mapping.button_id_to_pins[5] = {pins::kIndexMiddle};
  expected_mapping.button_id_to_pins[7] = {pins::kMiddleTop};
  expected_mapping.button_id_to_pins[11] = {pins::kMiddleMiddle};
  expected_mapping.button_id_to_pins[6] = {pins::kMiddleBottom};
  expected_mapping.button_id_to_pins[8] = {pins::kRingTop};
  expected_mapping.button_id_to_pins[12] = {pins::kRingMiddle};
  expected_mapping.button_id_to_pins[10] = {pins::kRingBottom};
  expected_mapping.button_id_to_pins[9] = {pins::kPinkyTop};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetButtonPinMapping_SpecialDigital) {
  hs_profile_Profile_Layer layer = {
      .thumb_top =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R_STICK_UP),
      .thumb_middle = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_R_STICK_DOWN),
      .thumb_bottom = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_R_STICK_LEFT),
      .index_top = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_R_STICK_RIGHT),
      .index_middle = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MIN),
      .middle_top = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX),
      .middle_middle = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MIN),
      .left_index_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN),
      .left_middle_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_UP),
      .left_ring_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MAX),
      .right_index_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT),
      .right_middle_extra = DigitalLayerAction(
          hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT),
      .right_ring_extra =
          DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_MOD)};

  const int joystick_min = 0;
  const int joystick_max = 1023;

  PCButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{joystick_max, pins::kThumbTop},
                          {joystick_min, pins::kThumbMiddle}};
  expected_mapping.z_x = {{joystick_min, pins::kThumbBottom},
                          {joystick_max, pins::kIndexTop}};
  expected_mapping.slider_left = {{joystick_min, pins::kIndexMiddle},
                                  {joystick_max, pins::kMiddleTop}};
  expected_mapping.slider_right = {{joystick_min, pins::kMiddleMiddle},
                                   {joystick_max, pins::kLeftRingExtra}};
  expected_mapping.hat_up = {pins::kLeftMiddleExtra};
  expected_mapping.hat_down = {pins::kLeftIndexExtra};
  expected_mapping.hat_left = {pins::kRightIndexExtra};
  expected_mapping.hat_right = {pins::kRightMiddleExtra};
  expected_mapping.mod = {pins::kRightRingExtra};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetButtonPinMapping_Analog) {
  hs_profile_Profile_Layer layer = {
      .thumb_top = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y, 100),
      .thumb_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X, 101),
      .pinky_middle = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT, 102),
      .pinky_bottom = AnalogLayerAction(
          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_RIGHT, 103)};

  PCButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{100, pins::kThumbTop}};
  expected_mapping.z_x = {{101, pins::kThumbMiddle}};
  expected_mapping.slider_left = {{102, pins::kPinkyMiddle}};
  expected_mapping.slider_right = {{103, pins::kPinkyBottom}};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetButtonPinMapping_MultiplePinsOneButton) {
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

  PCButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{100, pins::kThumbTop},
                          {101, pins::kThumbMiddle},
                          {102, pins::kThumbBottom}};
  expected_mapping.button_id_to_pins[2] = {pins::kIndexTop, pins::kIndexMiddle,
                                           pins::kMiddleTop};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetDPadAngle) {
  const uint8_t pin = 1;

  EXPECT_CALL(*teensy_, DigitalReadLow(pin)).WillRepeatedly(Return(true));

  PCController controller(std::move(teensy_));

  PCButtonPinMapping mapping;
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {.hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 90);

  mapping = {.hat_left = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 270);

  mapping = {.hat_left = {pin}, .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {.hat_down = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 180);

  mapping = {.hat_down = {pin}, .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 135);

  mapping = {.hat_down = {pin}, .hat_left = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 225);

  mapping = {.hat_down = {pin}, .hat_left = {pin}, .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 180);

  mapping = {.hat_up = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 0);

  mapping = {.hat_up = {pin}, .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 45);

  mapping = {.hat_up = {pin}, .hat_left = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 315);

  mapping = {.hat_up = {pin}, .hat_left = {pin}, .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 0);

  mapping = {.hat_up = {pin}, .hat_down = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {.hat_up = {pin}, .hat_down = {pin}, .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 90);

  mapping = {.hat_up = {pin}, .hat_down = {pin}, .hat_left = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), 270);

  mapping = {.hat_up = {pin},
             .hat_down = {pin},
             .hat_left = {pin},
             .hat_right = {pin}};
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);
}

TEST_F(PCControllerTest, UpdateButtons) {
  const uint8_t pin = 1;
  const std::vector<int> digital = {pin};
  const std::vector<AnalogButton> analog = {{.value = 0, .pin = pin}};
  PCButtonPinMapping mapping = {
      .z_y = analog,
      .z_x = analog,
      .slider_left = analog,
      .slider_right = analog,
      .hat_up = digital,
      .hat_down = digital,
      .hat_left = digital,
      .hat_right = digital,
  };
  mapping.mod = digital;
  mapping.button_id_to_pins = {{1, digital},  {2, digital},  {3, digital},
                               {4, digital},  {5, digital},  {6, digital},
                               {7, digital},  {8, digital},  {9, digital},
                               {10, digital}, {11, digital}, {12, digital}};

  EXPECT_CALL(*teensy_, DigitalReadLow(1))
      .Times(20)
      .WillRepeatedly(Return(true));

  {
    InSequence seq;
    EXPECT_CALL(*teensy_, SetJoystickZ);
    EXPECT_CALL(*teensy_, SetJoystickZRotate);
    EXPECT_CALL(*teensy_, SetJoystickSliderLeft);
    EXPECT_CALL(*teensy_, SetJoystickSliderRight);
    EXPECT_CALL(*teensy_, SetJoystickButton(_, true)).Times(12);
    EXPECT_CALL(*teensy_, SetJoystickHat);
  }

  PCController controller(std::move(teensy_));
  controller.UpdateButtons(mapping);
}

}  // namespace hs
