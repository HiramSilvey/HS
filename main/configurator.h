// Copyright 2024 Hiram Silvey

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <memory>

#include "teensy.h"

namespace hs {
namespace configurator {
namespace internal {

void FetchStoredBounds(const Teensy& teensy);
void FetchJoystickCoords(Teensy& teensy);
void CalibrateJoystick(Teensy& teensy);
void SaveCalibration(const Teensy& teensy);
void StoreProfiles(const Teensy& teensy);

}  // namespace internal

void Configure(std::unique_ptr<Teensy> teensy);

}  // namespace configurator
}  // namespace hs

#endif  // CONFIGURATOR_H_
