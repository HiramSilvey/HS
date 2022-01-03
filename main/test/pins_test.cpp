#include "pins.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "profile.pb.h"
#include "test/test_util.h"

namespace hs {

using ::testing::AllOf;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::Matcher;

std::vector<Matcher<pins::ActionPin>> ActionPinEq(
    const std::vector<pins::ActionPin>& expected) {
  std::vector<Matcher<pins::ActionPin>> matchers;
  for (const auto& action_pin : expected) {
    matchers.push_back(AllOf(
        Field("action", &pins::ActionPin::action, ActionEq(action_pin.action)),
        Field("pin", &pins::ActionPin::pin, action_pin.pin)));
  }
  return matchers;
}

TEST(PinsTest, GetActionPins) {
  const auto thumb_top =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_X);
  const auto thumb_middle =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_CIRCLE);
  const auto thumb_bottom =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_TRIANGLE);
  const auto index_top =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_SQUARE);
  const auto index_middle =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L1);
  const auto middle_top =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L2);
  const auto middle_middle =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_L3);
  const auto middle_bottom =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R1);
  const auto ring_top =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R2);
  const auto ring_middle =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_R3);
  const auto ring_bottom =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_OPTIONS);
  const auto pinky_top =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_SHARE);
  const auto pinky_middle =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_HOME);
  const auto pinky_bottom =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_CAPTURE);
  const auto left_outer =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_LEFT);
  const auto left_inner =
      DigitalLayerAction(hs_profile_Profile_Layer_DigitalAction_D_PAD_DOWN);

  const hs_profile_Profile_Layer layer = {.thumb_top = thumb_top,
                                          .thumb_middle = thumb_middle,
                                          .thumb_bottom = thumb_bottom,
                                          .index_top = index_top,
                                          .index_middle = index_middle,
                                          .middle_top = middle_top,
                                          .middle_middle = middle_middle,
                                          .middle_bottom = middle_bottom,
                                          .ring_top = ring_top,
                                          .ring_middle = ring_middle,
                                          .ring_bottom = ring_bottom,
                                          .pinky_top = pinky_top,
                                          .pinky_middle = pinky_middle,
                                          .pinky_bottom = pinky_bottom,
                                          .left_outer = left_outer,
                                          .left_inner = left_inner};

  const std::vector<pins::ActionPin> expected = {
      {thumb_top, pins::kThumbTop},
      {thumb_middle, pins::kThumbMiddle},
      {thumb_bottom, pins::kThumbBottom},
      {index_top, pins::kIndexTop},
      {index_middle, pins::kIndexMiddle},
      {middle_top, pins::kMiddleTop},
      {middle_middle, pins::kMiddleMiddle},
      {middle_bottom, pins::kMiddleBottom},
      {ring_top, pins::kRingTop},
      {ring_middle, pins::kRingMiddle},
      {ring_bottom, pins::kRingBottom},
      {pinky_top, pins::kPinkyTop},
      {pinky_middle, pins::kPinkyMiddle},
      {pinky_bottom, pins::kPinkyBottom},
      {left_outer, pins::kLeftOuter},
      {left_inner, pins::kLeftInner}};

  EXPECT_THAT(pins::GetActionPins(layer),
              ElementsAreArray(ActionPinEq(expected)));
}

}  // namespace hs
