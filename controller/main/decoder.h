// Copyright 2021 Hiram Silvey

#ifndef DECODER_H_
#define DECODER_H_

#include "profiles.pb.h"

class Decoder {
 public:
  static configurator_profiles_Profile_Layout Decode(configurator_profiles_Profile_Platform Platform, int position);
 private:
  std::vector<configurator_profiles_Profile_PlatformConfig> DecodeHeader(int addr);
  configurator_profiles_Profile_Layout DecodeBody(int addr);
};


#endif  // PROFILES_H_
