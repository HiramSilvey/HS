// Copyright 2021 Hiram Silvey

#include "decoder.h"

#include <EEPROM.h>

using Profile = configurator_profiles_Profile;
using Platform = configurator_profiles_Profile_Platform;
using PlatformConfig = configurator_profiles_Profile_PlatformConfig;
using PC = configurator_profiles_Profile_Platform_PC;
using Layout = configurator_profiles_Profile_Layout;

const kMinAddr = 16;

std::vector<PlatformConfig> Decoder::DecodeHeader(int addr) {
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

Layout Decoder::DecodeBody(int addr) {
  int remaining = 8;
  int body_len = EEPROM.read(addr++);
  return;
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
