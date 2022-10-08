// Copyright 2021 Hiram Silvey

// TODO: Convert joystick to wait for /INT and only read the value then. Use constant time step for filter at first, with the possibility of updating it to dynamic later if issues arise. Use cached joystick value when the value hasn't changed between loop runs.

#ifndef SNAPBACK_FILTER_H_
#define SNAPBACK_FILTER_H_

#include "math.h"

namespace hs {

// fmax = 3.3kHz according to the spec for Tlv493d MASTERCONTROLLEDMODE.
constexpr double kTlv493dUpdateInterval = 0.0003030303030303;

class SnapbackFilter {
 public:
  explicit SnapbackFilter(double frequency_cutoff)
      : alpha_(exp(-frequency_cutoff * kTlv493dUpdateInterval)) {}

  inline double Filter(double input) {
    output_ = (output_ - input) * alpha_ + input;
    return output_;
  }

 private:
  const double alpha_;
  double output_;
};

}  // namespace hs

#endif  // SNAPBACK_FILTER_H_
