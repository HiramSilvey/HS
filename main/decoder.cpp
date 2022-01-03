// Copyright 2021 Hiram Silvey

#include "decoder.h"

#include <memory>
#include <vector>

#include "profile.pb.h"
#include "teensy.h"

namespace hs {
namespace decoder {

using Platform = hs_profile_Profile_Platform;
using PlatformConfig = hs_profile_Profile_PlatformConfig;
using Layout = hs_profile_Profile_Layout;
using Layer = hs_profile_Profile_Layer;
using Action = hs_profile_Profile_Layer_Action;
using AnalogAction_ID = hs_profile_Profile_Layer_AnalogAction_ID;
using DigitalAction = hs_profile_Profile_Layer_DigitalAction;

const int kMinAddr = 12;
const int kMasks[10] = {
    0b1,      0b11,      0b111,      0b1111,      0b11111,
    0b111111, 0b1111111, 0b11111111, 0b111111111, 0b1111111111,
};
const int kLenActionID = 5;
const int kLenAnalogActionValue = 10;

namespace internal {

std::vector<PlatformConfig> DecodeHeader(const Teensy& teensy, int addr) {
  const uint8_t platform_bitmap = teensy.EEPROMRead(addr++);
  std::vector<PlatformConfig> configs;
  for (int platform = _hs_profile_Profile_Platform_MIN;
       platform <= _hs_profile_Profile_Platform_MAX; platform++) {
    if (platform_bitmap & (1 << (8 - platform))) {
      PlatformConfig config;
      config.platform = static_cast<Platform>(platform);
      if (configs.size() % 2 == 0) {
        config.position = teensy.EEPROMRead(addr) >> 4;
      } else {
        config.position = teensy.EEPROMRead(addr++) & 0xF;
      }
      configs.push_back(config);
    }
  }
  return configs;
}

int FetchData(const Teensy& teensy, int remaining, int& addr,
              uint8_t& curr_byte, int& unread) {
  int data = 0;
  while (remaining > 0) {
    int offset = unread - remaining;
    if (offset >= 0) {
      data |= (curr_byte >> offset) & kMasks[remaining - 1];
    } else {
      data |= (curr_byte << offset * -1) & kMasks[remaining - 1];
    }
    int fetched = unread < remaining ? unread : remaining;
    remaining -= fetched;
    unread -= fetched;
    if (unread == 0) {
      curr_byte = teensy.EEPROMRead(addr++);
      unread = 8;
    }
  }
  return data;
}

Layer DecodeLayer(const Teensy& teensy, int& addr) {
  Layer layer;
  Action* actions[20] = {&layer.thumb_top,          &layer.thumb_middle,
                         &layer.thumb_bottom,       &layer.index_top,
                         &layer.index_middle,       &layer.middle_top,
                         &layer.middle_middle,      &layer.middle_bottom,
                         &layer.ring_top,           &layer.ring_middle,
                         &layer.ring_bottom,        &layer.pinky_top,
                         &layer.pinky_middle,       &layer.pinky_bottom,
                         &layer.left_index_extra,   &layer.left_middle_extra,
                         &layer.left_ring_extra,    &layer.right_index_extra,
                         &layer.right_middle_extra, &layer.right_ring_extra};
  uint8_t curr_byte = teensy.EEPROMRead(addr++);
  int unread = 8;
  for (Action* action : actions) {
    int button_id = FetchData(teensy, kLenActionID, addr, curr_byte, unread);
    if (button_id > _hs_profile_Profile_Layer_DigitalAction_MAX) {
      action->action_type.analog.id = static_cast<AnalogAction_ID>(
          button_id - _hs_profile_Profile_Layer_DigitalAction_MAX);
      int button_value =
          FetchData(teensy, kLenAnalogActionValue, addr, curr_byte, unread);
      action->action_type.analog.value = button_value;
      action->which_action_type = hs_profile_Profile_Layer_Action_analog_tag;
    } else {
      action->action_type.digital = static_cast<DigitalAction>(button_id);
      action->which_action_type = hs_profile_Profile_Layer_Action_digital_tag;
    }
  }
  return layer;
}

Layout DecodeBody(const Teensy& teensy, int& addr) {
  const int max_addr = addr + teensy.EEPROMRead(addr++) + 1;

  Layout layout;
  layout.joystick_threshold = teensy.EEPROMRead(addr++);
  layout.base = DecodeLayer(teensy, addr);
  if (addr < max_addr) {
    layout.has_mod = true;
    layout.mod = DecodeLayer(teensy, addr);
  } else {
    layout.has_mod = false;
  }
  return layout;
}

}  // namespace internal

Layout Decode(const Teensy& teensy, Platform platform, int position) {
  int curr_addr = kMinAddr;
  const int encoded_len =
      (teensy.EEPROMRead(curr_addr++) << 8) | teensy.EEPROMRead(curr_addr++);
  const int max_addr = kMinAddr + encoded_len + 1;

  while (curr_addr < max_addr) {
    std::vector<PlatformConfig> configs =
        internal::DecodeHeader(teensy, curr_addr);
    // Advance past the header.
    curr_addr += configs.size() / 2 + configs.size() % 2 + 1;
    if ([&] {
          for (const auto& config : configs) {
            if (config.platform == platform && config.position == position) {
              return true;
            }
          }
          return false;
        }()) {
      break;
    }
    // Advance to the header of the next profile.
    curr_addr += teensy.EEPROMRead(curr_addr++);
  }

  if (curr_addr >= max_addr) {
    teensy.Exit(1);
  }

  return internal::DecodeBody(teensy, curr_addr);
}

}  // namespace decoder
}  // namespace hs
