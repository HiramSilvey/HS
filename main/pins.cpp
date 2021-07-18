// Copyright 2021 Hiram Silvey

#include "pins.h"

#include "Arduino.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;

std::vector<Pins::ActionPin> Pins::GetActionPins(const Layout& layout) {
  return {
          {layout.thumb_top, kThumbTop},
          {layout.thumb_middle, kThumbMiddle},
          {layout.thumb_bottom, kThumbBottom},
          {layout.index_top, kIndexTop},
          {layout.index_middle, kIndexMiddle},
          {layout.middle_top, kMiddleTop},
          {layout.middle_middle, kMiddleMiddle},
          {layout.middle_bottom, kMiddleBottom},
          {layout.ring_top, kRingTop},
          {layout.ring_middle, kRingMiddle},
          {layout.ring_bottom, kRingBottom},
          {layout.pinky_top, kPinkyTop},
          {layout.pinky_middle, kPinkyMiddle},
          {layout.pinky_bottom, kPinkyBottom},
          {layout.left_ring_extra, kLeftRingExtra},
          {layout.left_middle_extra, kLeftMiddleExtra},
          {layout.left_index_extra, kLeftIndexExtra},
          {layout.right_index_extra, kRightIndexExtra},
          {layout.right_middle_extra, kRightMiddleExtra},
          {layout.right_ring_extra, kRightRingExtra}
  };
}
