// Copyright 2021 Hiram Silvey

#ifndef PROFILES_H_
#define PROFILES_H_

#include "profiles.pb.h"

class Profiles {
 public:
  static void Store();
  static configurator_profiles_Profile Fetch(configurator_profiles_Profile_Platform Platform);
 private:
  int GetPosition();
};


#endif  // PROFILES_H_
