// Copyright 2021 Hiram Silvey

#ifndef DECODER_H_
#define DECODER_H_

#include "profile.pb.h"

class Decoder {
public:
  static hs_profile_Profile_Layout Decode(hs_profile_Profile_Platform Platform, int position);
};


#endif  // PROFILES_H_
