#include "pc_controller.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "controller.h"
#include "mock_teensy.h"
#include "pins.h"
#include "profile.pb.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

bool operator ==(const AnalogButton& lhs, const AnalogButton& rhs) {
  return lhs.value == rhs.value && lhs.pin == rhs.pin;
}

bool operator ==(const PCButtonPinMapping& lhs, const PCButtonPinMapping& rhs) {
  return lhs.button_id_to_pins == rhs.button_id_to_pins
    && lhs.mod == rhs.mod
    && lhs.z_y == rhs.z_y && lhs.z_x == rhs.z_x
    && lhs.slider_left == rhs.slider_left
    && lhs.slider_right == rhs.slider_right
    && lhs.hat_up == rhs.hat_up
    && lhs.hat_down == rhs.hat_down
    && lhs.hat_left == rhs.hat_left
    && lhs.hat_right == rhs.hat_right;
}

class PCControllerTest : public ::testing::Test {
protected:
  PCControllerTest() {
    teensy_ = std::make_unique<MockTeensy>();

    EXPECT_CALL(*teensy_, DigitalReadLow)
      .Times(AtLeast(1));
    EXPECT_CALL(*teensy_, EEPROMRead)
      .Times(AtLeast(1));
    EXPECT_CALL(*teensy_, Exit);
    EXPECT_CALL(*teensy_, JoystickUseManualSend);
  }
  std::unique_ptr<MockTeensy> teensy_;
};

TEST_F(PCControllerTest, GetButtonPinMappingStandardDigital) {
  hs_profile_Profile_Layer layer = {
    .thumb_top = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_X,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .thumb_middle = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_CIRCLE,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .thumb_bottom = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_TRIANGLE,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .index_top = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_SQUARE,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .index_middle = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_L1,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .middle_top = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_L2,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .middle_middle = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_L3,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .middle_bottom = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_R1,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .ring_top = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_R2,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .ring_middle = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_R3,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .ring_bottom = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_OPTIONS,
      .which_action_type = hs_profile_Profile_Layer_Action_digital_tag,
    },
    .pinky_top = {
      .action_type.digital = hs_profile_Profile_Layer_DigitalAction_SHARE,
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

  EXPECT_EQ(controller.GetButtonPinMapping(layer), expected_mapping);
}

TEST_F(PCControllerTest, GetButtonPinMappingAnalog) {
  hs_profile_Profile_Layer layer = {
    .thumb_top = {
      .action_type.analog = {
        .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_Y,
        .value = 100,
      },
      .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
    },
    .thumb_middle = {
      .action_type.analog = {
        .id = hs_profile_Profile_Layer_AnalogAction_ID_R_STICK_X,
        .value = 101,
      },
      .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
    },
    .pinky_middle = {
      .action_type.analog = {
        .id = hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_LEFT,
        .value = 102,
      },
      .which_action_type = hs_profile_Profile_Layer_Action_analog_tag,
    },
    .pinky_bottom = {
      .action_type.analog = {
        .id = hs_profile_Profile_Layer_AnalogAction_ID_SLIDER_RIGHT,
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

  EXPECT_EQ(controller.GetButtonPinMapping(layer), expected_mapping);
}

TEST_F(PCControllerTest, GetDPadAngle) {
  const uint8_t pin = 1;

  EXPECT_CALL(*teensy_, DigitalReadLow(pin))
    .WillRepeatedly(Return(true));

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
