// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include <memory>

#include "teensy.h"

namespace hs {
namespace configurator {

void StoreProfiles(Teensy& teensy) {
  while (teensy.SerialAvailable() < 2) {
  }
  int num_bytes = teensy.SerialRead() << 8 | teensy.SerialRead();
  int curr_address = 0;
  while (curr_address < num_bytes) {
    if (teensy.SerialAvailable()) {
      uint8_t data = teensy.SerialRead();
      teensy.EEPROMUpdate(curr_address, data);
      curr_address++;
    }
  }
  teensy.SerialWrite(0);  // Done.
}

}  // namespace configurator
}  // namespace hs
