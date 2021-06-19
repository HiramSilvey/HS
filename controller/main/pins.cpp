#include "pins.h"

#include "Arduino.h"
#include "profile.pb.h"

using Layout = hs_profile_Profile_Layout;

void Pins::Init() {
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
}

std::vector<ActionPin> Pins::GetActionPins(const Layout& layout) {
  return {
    {layout.thumb_top, 0},
    {layout.thumb_middle, 1},
    {layout.thumb_bottom, 2},
    {layout.index_top, 3},
    {layout.index_middle, 4},
    {layout.middle_top, 5},
    {layout.middle_middle, 6},
    {layout.middle_bottom, 7},
    {layout.ring_top, 8},
    {layout.ring_middle, 9},
    {layout.ring_bottom, 10},
    {layout.pinky_top, 11},
    {layout.pinky_middle, 12},
    {layout.pinky_bottom, 13},
    {layout.left_ring_extra, 14},
    {layout.left_middle_extra, 15},
    {layout.left_index_extra, 16},
    {layout.right_index_extra, 17},
    {layout.right_middle_extra, 20},
    {layout.right_ring_extra, 21},
  };
}
