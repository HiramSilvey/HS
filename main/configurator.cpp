// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include <memory>

#include "teensy.h"
#include "util.h"

void WriteToSerial(const std::unique_ptr<Teensy>& teensy, int val) {
  uint8_t bytes[4] = {val >> 24, val >> 16 & 0xFF, val >> 8 & 0xFF, val & 0xFF};
  teensy->SerialWrite(bytes, 4);
}

void SaveToEEPROM(const std::unique_ptr<Teensy>& teensy, int val, int address) {
  uint8_t one = val >> 24;
  uint8_t two = val >> 16 & 0xFF;
  uint8_t three = val >> 8 & 0xFF;
  uint8_t four = val & 0xFF;

  teensy->EEPROMUpdate(address, one);
  teensy->EEPROMUpdate(address + 1, two);
  teensy->EEPROMUpdate(address + 2, three);
  teensy->EEPROMUpdate(address + 3, four);
}

void Configurator::FetchStoredBounds(const std::unique_ptr<Teensy>& teensy) {
  int center_x = Util::GetIntFromEEPROM(teensy, 0);
  int center_y = Util::GetIntFromEEPROM(teensy, 4);
  int range = Util::GetIntFromEEPROM(teensy, 8);

  WriteToSerial(teensy, center_x);
  WriteToSerial(teensy, center_y);
  WriteToSerial(teensy, range);
}

void Configurator::FetchJoystickCoords(const std::unique_ptr<Teensy>& teensy) {
  teensy->UpdateHallData();
  float z = teensy->GetHallZ();
  int x = teensy->GetHallX() / z * 1000000;
  int y = teensy->GetHallY() / z * 1000000;

  WriteToSerial(teensy, x);
  WriteToSerial(teensy, y);
}

void Configurator::CalibrateJoystick(const std::unique_ptr<Teensy>& teensy) {
  float min_x = 0;
  float max_x = 0;
  float min_y = 0;
  float max_y = 0;

  uint64_t end_time = teensy->Millis() + 15000;  // 15 seconds from now.
  while (teensy->Millis() < end_time) {
    teensy->UpdateHallData();
    float z = teensy->GetHallZ();
    float x = teensy->GetHallX() / z;
    float y = teensy->GetHallY() / z;

    if (x < min_x) {
      min_x = x;
    } else if (x > max_x) {
      max_x = x;
    }
    if (y < min_y) {
      min_y = y;
    } else if (y > max_y) {
      max_y = y;
    }
  }

  float range_x = (max_x - min_x) / 2.0;
  float range_y = (max_y - min_y) / 2.0;

  int center_x = (min_x + range_x) * 1000000;
  int center_y = (min_y + range_y) * 1000000;
  int range = range_x >= range_y ? range_x * 1000000 : range_y * 1000000;

  WriteToSerial(teensy, center_x);
  WriteToSerial(teensy, center_y);
  WriteToSerial(teensy, range);
}

void Configurator::SaveCalibration(const std::unique_ptr<Teensy>& teensy) {
  int bytes_to_read = 12;
  int address = 0;
  while (address < bytes_to_read) {
    if (teensy->SerialAvailable()) {
      teensy->EEPROMUpdate(address, teensy->SerialRead());
      address++;
    }
  }
  teensy->SerialWrite(0);  // Done.
}

void Configurator::StoreProfiles(const std::unique_ptr<Teensy>& teensy) {
  while (teensy->SerialAvailable() < 2) {
  }
  int num_bytes = teensy->SerialRead() << 8 | teensy->SerialRead();
  int base_address = 12;  // 0-11 are reserved for joystick calibration values.
  int curr_address = base_address;
  while (curr_address < base_address + num_bytes) {
    if (teensy->SerialAvailable()) {
      uint8_t data = teensy->SerialRead();
      teensy->EEPROMUpdate(curr_address, data);
      curr_address++;
    }
  }
  teensy->SerialWrite(0);  // Done.
}

void Configurator::Configure(std::unique_ptr<Teensy> teensy) {
  while (true) {
    if (teensy->SerialAvailable() > 0) {
      uint8_t data = teensy->SerialRead();
      if (data > 4) {
        teensy->SerialWrite(1);  // Error.
        continue;
      }
      teensy->SerialWrite(0);  // OK.
      switch (data) {
        case 0:
          FetchStoredBounds(teensy);
          break;
        case 1:
          FetchJoystickCoords(teensy);
          break;
        case 2:
          CalibrateJoystick(teensy);
          break;
        case 3:
          SaveCalibration(teensy);
          break;
        case 4:
          StoreProfiles(teensy);
          break;
      }
    }
  }
}
