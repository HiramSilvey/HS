// Copyright 2021 Hiram Silvey

#ifndef DECODER_H_
#define DECODER_H_

#include "profiles.pb.h"

class Decoder {
 public:
  configurator_profiles_Profile Decode(configurator_profiles_Profile_Platform Platform, int position);
 private:
  int FindAddress(configurator_profiles_Profile_Platform Platform, int position);
  int DecodeHeader(int address, configurator_profiles_Profile_PlatformConfig& platform_config);

  const int data_len_;
  int curr_addr_;
};


#endif  // PROFILES_H_
