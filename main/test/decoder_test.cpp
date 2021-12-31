#include "decoder.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "profile.pb.h"
#include "test_util.h"

namespace hs {

using ::testing::AllOf;
using ::testing::Field;

using Layout = ::hs_profile_Profile_Layout;
using Layer = ::hs_profile_Profile_Layer;

auto LayerEq(const Layer& expected) {
  return AllOf(
      Field(Layer::has_thumb_top, expected.has_thumb_top),
      Field(Layer::thumb_top, ActionEq(expected.thumb_top)),
      Field(Layer::has_thumb_middle, expected.has_thumb_middle),
      Field(Layer::thumb_middle, ActionEq(expected.thumb_middle)),
      Field(Layer::has_thumb_bottom, expected.has_thumb_bottom),
      Field(Layer::thumb_bottom, ActionEq(expected.thumb_bottom)),
      Field(Layer::has_index_top, expected.has_index_top),
      Field(Layer::index_top, ActionEq(expected.index_top)),
      Field(Layer::has_index_middle, expected.has_index_middle),
      Field(Layer::index_middle, ActionEq(expected.index_middle)),
      Field(Layer::has_middle_top, expected.has_middle_top),
      Field(Layer::middle_top, ActionEq(expected.middle_top)),
      Field(Layer::has_middle_middle, expected.has_middle_middle),
      Field(Layer::middle_middle, ActionEq(expected.middle_middle)),
      Field(Layer::has_middle_bottom, expected.has_middle_bottom),
      Field(Layer::middle_bottom, ActionEq(expected.middle_bottom)),
      Field(Layer::has_ring_top, expected.has_ring_top),
      Field(Layer::ring_top, ActionEq(expected.ring_top)),
      Field(Layer::has_ring_middle, expected.has_ring_middle),
      Field(Layer::ring_middle, ActionEq(expected.ring_middle)),
      Field(Layer::has_ring_bottom, expected.has_ring_bottom),
      Field(Layer::ring_bottom, ActionEq(expected.ring_bottom)),
      Field(Layer::has_pinky_top, expected.has_pinky_top),
      Field(Layer::pinky_top, ActionEq(expected.pinky_top)),
      Field(Layer::has_pinky_middle, expected.has_pinky_middle),
      Field(Layer::pinky_middle, ActionEq(expected.pinky_middle)),
      Field(Layer::has_pinky_bottom, expected.has_pinky_bottom),
      Field(Layer::pinky_bottom, ActionEq(expected.pinky_bottom)),
      Field(Layer::has_left_index_extra, expected.has_left_index_extra),
      Field(Layer::left_index_extra, ActionEq(expected.left_index_extra)),
      Field(Layer::has_left_middle_extra, expected.has_left_middle_extra),
      Field(Layer::left_middle_extra, ActionEq(expected.left_middle_extra)),
      Field(Layer::has_left_ring_extra, expected.has_left_ring_extra),
      Field(Layer::left_ring_extra, ActionEq(expected.left_ring_extra)),
      Field(Layer::has_right_index_extra, expected.has_right_index_extra),
      Field(Layer::right_index_extra, ActionEq(expected.right_index_extra)),
      Field(Layer::has_right_middle_extra, expected.has_right_middle_extra),
      Field(Layer::right_middle_extra, ActionEq(expected.right_middle_extra)),
      Field(Layer::has_right_ring_extra, expected.has_right_ring_extra),
      Field(Layer::right_ring_extra, ActionEq(expected.right_ring_extra)));
}

auto LayoutEq(const Layout& expected) {
  return AllOf(Field(Layout::joystick_threshold, expected.joystick_threshold),
               Field(Layout::has_base, expected.has_base),
               Field(Layout::base, LayerEq(expected.base)),
               Field(Layout::has_mod, expected.has_mod),
               Field(Layout::mod, LayerEq(expected.mod)));
}

}  // namespace hs
