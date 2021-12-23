// Copyright 2021 Hiram Silvey

#ifndef DECODER_H_
#define DECODER_H_

#include <memory>

#include "profile.pb.h"
#include "teensy.h"

class Decoder {
 public:
  static hs_profile_Profile_Layout Decode(const std::unique_ptr<Teensy>& teensy,
                                          hs_profile_Profile_Platform Platform,
                                          int position);
};

#endif  // PROFILES_H_
