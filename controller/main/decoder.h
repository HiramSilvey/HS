// Copyright 2021 Hiram Silvey

#ifndef DECODER_H_
#define DECODER_H_

#include "profiles.pb.h"

class Decoder {
 public:
  static int GetAddress(configurator_profiles_Profile_Platform Platform, int position);
  static configurator_profiles_Profile Decode(int address);
 private:
  configurator_profiles_Profile_PlatformConfig GetPlatformConfig(int address);
  int GetNextAddress(int address);
};


#endif  // PROFILES_H_
