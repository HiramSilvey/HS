// Copyright 2021 Hiram Silvey

#ifndef PROFILES_H_
#define PROFILES_H_

#include "profile.pb.h"

class Profiles {
 public:
  static void Store();
  static hs_profile_Profile_Layout Fetch(hs_profile_Profile_Platform Platform);
};


#endif  // PROFILES_H_
