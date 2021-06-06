// Copyright 2021 Hiram Silvey

#include "decoder.h"

#include <EEPROM.h>

using Profile = configurator_profiles_Profile;
using Platform = configurator_profiles_Profile_Platform;
using PlatformConfig = configurator_profiles_Profile_PlatformConfig;
using PC = configurator_profiles_Profile_Platform_PC;
using Layout = configurator_profiles_Profile_Layout;
using Action = configurator_profiles_Profile_Layout_Action;
using DigitalAction = configurator_profiles_Profile_Layout_DigitalAction;
using AnalogAction_ID = configurator_profiles_Profile_Layout_AnalogAction_ID;

const int kMinAddr = 16;
const int kMasks[8] = {
                       0b1,
                       0b11,
                       0b111,
                       0b1111,
                       0b11111,
                       0b111111,
                       0b1111111,
                       0b11111111,
};
const int kLenButtonID = 5;
const int kLenAnalogButtonValue = 10;
const int kMinAnalogActionID = configurator_profiles_Profile_Layout_AnalogAction_ID.configurator_profiles_Profile_Layout_AnalogAction_ID_R_STICK_Y;

std::vector<PlatformConfig> DecodeHeader(int addr) {
  const byte platform_bitmap = EEPROM.read(addr++);
  std::vector<PlatformConfig> configs;
  for (int platform = PC /*first*/; platform <= PC /*last*/; platform++) {
    if (platform_bitmap & 1 << 8 - platform) {
      PlatformConfig config;
      config.platform = static_cast<Platform>(platform);
      if (configs.size() % 2 == 0) {
        config.position = EEPORM.read(addr) >> 4;
      } else {
        config.position = EEPROM.read(addr++) & 0xFF;
      }
      configs.push_back(config);
    }
  }
  return configs;
}

int FetchData(int remaining, int& addr, byte& curr_byte, int& unread) {
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
      curr_byte = EEPROM.read(addr++);
      unread = 8;
    }
  }
  return data;
}

Layout DecodeBody(int addr) {
  Layout layout;
  Action& actions[20] = {
                         layout.thumb_top,
                         layout.thumb_middle,
                         layout.thumb_bottom,
                         layout.index_top,
                         layout.index_middle,
                         layout.middle_top,
                         layout.middle_middle,
                         layout.middle_bottom,
                         layout.ring_top,
                         layout.ring_middle,
                         layout.ring_bottom,
                         layout.pinky_top,
                         layout.pinky_middle,
                         layout.pinky_bottom,
                         layout.left_index_extra,
                         layout.left_middle_extra,
                         layout.left_ring_extra,
                         layout.right_index_extra,
                         layout.right_middle_extra,
                         layout.right_ring_extra
  }
  byte curr_byte = EEPROM.read(addr++);
  int unread = 8;
  for (Action& action : actions) {
    int button_id = FetchData(kLenButtonID, addr, curr_byte, unread);
    if (button_id >= kMinAnalogActionID) {
      action.action_type.analog.id = static_cast<AnalogAction_ID>(button_id);
      int button_value = FetchData(kLenAnalogActionValue, addr, curr_byte, unread);
      action.action_type.analog.value = button_value;
    } else {
      action.action_type.digital = static_cast<DigitalAction>(button_id);
    }
  }
  return layout;
}

Layout Decoder::Decode(Platform platform, int position) {
  const int max_addr = kMinAddr + EEPROM.read(kMinAddr) + 1;

  int curr_addr = kMinAddr + 1;
  while (curr_addr < max_addr) {
    std::vector<PlatformConfig> configs = DecodeHeader(curr_addr);
    // Advance past the header.
    curr_addr += configs.size() / 2 + configs.size() % 2 + 1;
    if ([&]{
          for (const auto& config : configs) {
            if (config.platform == platform && config.position == position) {
              return true;
            }
          }
          return false;
        }) {
      break;
    }
    // Advance to the header of the next profile.
    curr_addr += EEPROM.read(curr_addr++);
  }
  if (curr_addr >= max_addr) {
    throw std::runtime_error("No matching or default profile found for the specified platform!"); 
  }

  return DecodeBody(curr_addr);
}
