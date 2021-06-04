// Copyright 2021 Hiram Silvey

#include "decoder.h"

using Profile = configurator_profiles_Profile;
using Platform = configurator_profiles_Profile_Platform;
using PlatformConfig = configurator_profiles_Profile_PlatformConfig;

const kLowestAddr = 16;

int Decoder::GetAddress(Platform platform, int position) {
  // If not found, fall back to default.
  int curr_address = kLowestAddr;
  int default_address = 0;
  while (true) {
    PlatformConfig platform_config = GetPlatformConfig(address);
    if (platform_config.platform == platform) {
      if (platform_config.position == position) {
        return address;
      }
      // Default position.
      if (platform_config.position == 0) {
        default_address = address;
      }
    }
    curr_address = GetNextAddress(address);
    if (curr_address < kLowestAddr) {
      if (default_address >= kLowestAddr) {
        return default_address;
      }
      throw std::runtime_error("No matching or default profile found for the specified platform!");
    }
  }
}
