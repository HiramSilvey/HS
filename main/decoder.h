// Copyright 2021 Hiram Silvey

#ifndef DECODER_H_
#define DECODER_H_

#include <memory>
#include <vector>

#include "profile.pb.h"
#include "teensy.h"

namespace hs {
namespace decoder {

namespace internal {

std::vector<hs_profile_Profile_PlatformConfig> DecodeHeader(
    const std::unique_ptr<Teensy>& teensy, int addr);
int FetchData(const std::unique_ptr<Teensy>& teensy, int remaining, int& addr,
              uint8_t& curr_byte, int& unread);
hs_profile_Profile_Layer DecodeLayer(const std::unique_ptr<Teensy>& teensy,
                                     int& addr);
hs_profile_Profile_Layout DecodeBody(const std::unique_ptr<Teensy>& teensy,
                                     int& addr);

}  // namespace internal

hs_profile_Profile_Layout Decode(const std::unique_ptr<Teensy>& teensy,
                                 hs_profile_Profile_Platform Platform,
                                 int position);

}  // namespace decoder
}  // namespace hs

#endif  // PROFILES_H_
