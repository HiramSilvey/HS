#include "pc_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "controller.h"
#include "mock_teensy.h"
#include "pins.h"
#include "profile.pb.h"

namespace hs {

using ::testing::_;
using ::testing::AllOf;
using ::testing::AtLeast;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::Matcher;
using ::testing::Return;

std::vector<Matcher<AnalogButton>> AnalogEq(
    const std::vector<AnalogButton>& expected) {
  std::vector<Matcher<AnalogButton>> matchers;
  for (const auto& button : expected) {
    matchers.push_back(AllOf(Field(&AnalogButton::value, button.value),
                             Field(&AnalogButton::pin, button.pin)));
  }
  return matchers;
}

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
      .thumb_top =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_X,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .thumb_middle =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_CIRCLE,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .thumb_bottom =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_TRIANGLE,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .index_top =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_SQUARE,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .index_middle =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_L1,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .middle_top =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_L2,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .middle_middle =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_L3,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .middle_bottom =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_R1,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .ring_top =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_R2,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .ring_middle =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_R3,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .ring_bottom =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_OPTIONS,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .pinky_top =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_SHARE,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
  };

  PCButtonPinMapping expected_mapping;
  expected_mapping.button_id_to_pins[2] = {kThumbTop};
  expected_mapping.button_id_to_pins[3] = {kThumbMiddle};
  expected_mapping.button_id_to_pins[4] = {kThumbBottom};
  expected_mapping.button_id_to_pins[1] = {kIndexTop};
  expected_mapping.button_id_to_pins[5] = {kIndexMiddle};
  expected_mapping.button_id_to_pins[7] = {kMiddleTop};
  expected_mapping.button_id_to_pins[11] = {kMiddleMiddle};
  expected_mapping.button_id_to_pins[6] = {kMiddleBottom};
  expected_mapping.button_id_to_pins[8] = {kRingTop};
  expected_mapping.button_id_to_pins[12] = {kRingMiddle};
  expected_mapping.button_id_to_pins[10] = {kRingBottom};
  expected_mapping.button_id_to_pins[9] = {kPinkyTop};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetButtonPinMapping_SpecialDigital) {
  hs_profile_Profile_Layer layer = {
      .thumb_top =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_R_STICK_UP,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .thumb_middle =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_R_STICK_DOWN,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .thumb_bottom =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_R_STICK_LEFT,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .index_top =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_R_STICK_RIGHT,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .index_middle =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MIN,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .middle_top =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_SLIDER_LEFT_MAX,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .middle_middle =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MIN,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .left_ring_extra =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_SLIDER_RIGHT_MAX,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .left_middle_extra =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_D_PAD_UP,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .left_index_extra =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .right_index_extra =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .right_middle_extra =
          {
              .action_type.digital =
                  hs_profile_Profile_Layer_DigitalAction_D_PAD_RIGHT,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .right_ring_extra =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_MOD,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
  };

  const int joystick_min = 0;
  const int joystick_max = 1023;

  PCButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{joystick_max, kThumbTop},
                          {joystick_min, kThumbMiddle}};
  expected_mapping.z_x = {{joystick_min, kThumbBottom},
                          {joystick_max, kIndexTop}};
  expected_mapping.slider_left = {{joystick_min, kIndexMiddle},
                                  {joystick_max, kMiddleTop}};
  expected_mapping.slider_right = {{joystick_min, kMiddleMiddle},
                                   {joystick_max, kLeftRingExtra}};
  expected_mapping.hat_up = {kLeftMiddleExtra};
  expected_mapping.hat_down = {kLeftIndexExtra};
  expected_mapping.hat_left = {kRightIndexExtra};
  expected_mapping.hat_right = {kRightMiddleExtra};
  expected_mapping.mod = {kRightRingExtra};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetButtonPinMapping_Analog) {
  hs_profile_Profile_Layer layer = {
      .thumb_top =
          {
              .action_type.analog =
                  {
                      .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y,
                      .value = 100,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
      .thumb_middle =
          {
              .action_type.analog =
                  {
                      .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X,
                      .value = 101,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
      .pinky_middle =
          {
              .action_type.analog =
                  {
                      .id =
                          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT,
                      .value = 102,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
      .pinky_bottom =
          {
              .action_type.analog =
                  {
                      .id =
                          hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_RIGHT,
                      .value = 103,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
  };

  PCButtonPinMapping expected_mapping;
  expected_mapping.z_y = {{100, kThumbTop}};
  expected_mapping.z_x = {{101, kThumbMiddle}};
  expected_mapping.slider_left = {{102, kPinkyMiddle}};
  expected_mapping.slider_right = {{103, kPinkyBottom}};

  PCController controller(std::move(teensy_));
  EXPECT_THAT(controller.GetButtonPinMapping(layer),
              MappingEq(expected_mapping));
}

TEST_F(PCControllerTest, GetButtonPinMapping_MultiplePinsOneButton) {
  hs_profile_Profile_Layer layer = {
      .thumb_top =
          {
              .action_type.analog =
                  {
                      .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y,
                      .value = 100,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
      .thumb_middle =
          {
              .action_type.analog =
                  {
                      .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y,
                      .value = 101,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
      .thumb_bottom =
          {
              .action_type.analog =
                  {
                      .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y,
                      .value = 102,
                  },
              .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
          },
      .index_top =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_X,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .index_middle =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_X,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
      .middle_top =
          {
              .action_type.digital = hs_profile_Profile_Layer_DigitalAction_X,
              .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
          },
  };

  PCButtonPinMapping expected_mapping;
  expected_mapping.z_y = {
      {100, kThumbTop}, {101, kThumbMiddle}, {102, kThumbBottom}};
  expected_mapping.button_id_to_pins[2] = {kIndexTop, kIndexMiddle, kMiddleTop};

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

  mapping = {
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 90);

  mapping = {
      .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 270);

  mapping = {
      .hat_left = {pin},
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {
      .hat_down = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 180);

  mapping = {
      .hat_down = {pin},
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 135);

  mapping = {
      .hat_down = {pin},
      .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 225);

  mapping = {
      .hat_down = {pin},
      .hat_left = {pin},
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 180);

  mapping = {
      .hat_up = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 0);

  mapping = {
      .hat_up = {pin},
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 45);

  mapping = {
      .hat_up = {pin},
      .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 315);

  mapping = {
      .hat_up = {pin},
      .hat_left = {pin},
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 0);

  mapping = {
      .hat_up = {pin},
      .hat_down = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), -1);

  mapping = {
      .hat_up = {pin},
      .hat_down = {pin},
      .hat_right = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 90);

  mapping = {
      .hat_up = {pin},
      .hat_down = {pin},
      .hat_left = {pin},
  };
  EXPECT_EQ(controller.GetDPadAngle(mapping), 270);

  mapping = {
      .hat_up = {pin},
      .hat_down = {pin},
      .hat_left = {pin},
      .hat_right = {pin},
  };
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
