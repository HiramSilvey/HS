// Copyright 2021 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <memory>

#include "teensy.h"

namespace hs {
namespace configurator {
namespace internal {

void FetchStoredBounds(const std::unique_ptr<Teensy>& teensy);
void FetchJoystickCoords(const std::unique_ptr<Teensy>& teensy);
void CalibrateJoystick(const std::unique_ptr<Teensy>& teensy);
void SaveCalibration(const std::unique_ptr<Teensy>& teensy);
void StoreProfiles(const std::unique_ptr<Teensy>& teensy);

}  // namespace internal

void Configure(std::unique_ptr<Teensy> teensy);

}  // namespace configurator
}  // namespace hs

#endif  // CONFIGURATOR_H_
