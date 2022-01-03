// Copyright 2021 Hiram Silvey

#include "pins.h"

#include "profile.pb.h"

namespace hs {
namespace pins {

std::vector<ActionPin> GetActionPins(const hs_profile_Profile_Layer& layer) {
  return {{layer.thumb_top, kThumbTop},
          {layer.thumb_middle, kThumbMiddle},
          {layer.thumb_bottom, kThumbBottom},
          {layer.index_top, kIndexTop},
          {layer.index_middle, kIndexMiddle},
          {layer.middle_top, kMiddleTop},
          {layer.middle_middle, kMiddleMiddle},
          {layer.middle_bottom, kMiddleBottom},
          {layer.ring_top, kRingTop},
          {layer.ring_middle, kRingMiddle},
          {layer.ring_bottom, kRingBottom},
          {layer.pinky_top, kPinkyTop},
          {layer.pinky_middle, kPinkyMiddle},
          {layer.pinky_bottom, kPinkyBottom},
          {layer.left_ring_extra, kLeftRingExtra},
          {layer.left_middle_extra, kLeftMiddleExtra},
          {layer.left_index_extra, kLeftIndexExtra},
          {layer.right_index_extra, kRightIndexExtra},
          {layer.right_middle_extra, kRightMiddleExtra},
          {layer.right_ring_extra, kRightRingExtra}};
}

}  // namespace pins
}  // namespace hs
