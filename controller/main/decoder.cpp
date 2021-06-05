// Copyright 2021 Hiram Silvey

#include "decoder.h"

#include <EEPROM.h>

using Profile = configurator_profiles_Profile;
using Platform = configurator_profiles_Profile_Platform;
using PlatformConfig = configurator_profiles_Profile_PlatformConfig;

const kLowestAddr = 16;

Decoder::Decoder() : data_len_(EEPROM.read(kLowestAddr)) {
  curr_addr_ = kLowestAddr + 1;
}

std::vector<PlatformConfig> Decoder::DecodeHeader() {
  const byte platform_bitmap = EEPROM.read(curr_addr_++);
  std::vector<Platform> platforms;
}

int Decoder::FindAddress(Platform platform, int position) {
  // If not found, fall back to default.
  int default_addr = 0;
  while (curr_addr_ < kLowestAddr + data_len_ + 1) {
    PlatformConfig platform_config;
    int header_len = DecodeHeader(address, &platform_config);
    if (header_len <= 0) {
      break;
    }
    curr_addr_ += header_len;
    if (platform_config.platform == platform) {
      if (platform_config.position == position) {
        return curr_addr_;
      }
      // Default position.
      if (platform_config.position == 0) {
        default_address = address;
      }
    }
    // Advance to the next profile.
    int body_len = EEPROM.read(curr_addr_++);
    curr_addr_ += body_len;
    header_len = DecodeHeader(address, &platform_config);
  }

  if (default_address >= kLowestAddr) {
    return default_address;
  }
  throw std::runtime_error("No matching or default profile found for the specified platform!");
}

Profile Decoder::Decode(Platform platform, int position) {
  int address = FindAddress(platform, position);
  int body_len = EEPROM.read(address);
  
}
