// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include <memory>

#include "mcu.h"
#include "util.h"

void WriteToSerial(const std::unique_ptr<MCU>& mcu, int val) {
  uint8_t bytes[4] = {val >> 24,
                      val >> 16 & 0xFF,
                      val >> 8 & 0xFF,
                      val & 0xFF};
  mcu->SerialWrite(bytes, 4);
}

void SaveToEEPROM(const std::unique_ptr<MCU>& mcu, int val, int address) {
  uint8_t one = val >> 24;
  uint8_t two = val >> 16 & 0xFF;
  uint8_t three = val >> 8 & 0xFF;
  uint8_t four = val & 0xFF;

  mcu->EEPROMUpdate(address, one);
  mcu->EEPROMUpdate(address+1, two);
  mcu->EEPROMUpdate(address+2, three);
  mcu->EEPROMUpdate(address+3, four);
}

void Configurator::FetchStoredBounds(const std::unique_ptr<MCU>& mcu) {
  int center_x = Util::GetIntFromEEPROM(mcu, 0);
  int center_y = Util::GetIntFromEEPROM(mcu, 4);
  int range = Util::GetIntFromEEPROM(mcu, 8);

  WriteToSerial(mcu, center_x);
  WriteToSerial(mcu, center_y);
  WriteToSerial(mcu, range);
}

void Configurator::FetchJoystickCoords(const std::unique_ptr<MCU>& mcu) {
  mcu->UpdateHallData();
  float z = mcu->GetHallZ();
  int x = mcu->GetHallX() / z * 1000000;
  int y = mcu->GetHallY() / z * 1000000;

  WriteToSerial(mcu, x);
  WriteToSerial(mcu, y);
}

void Configurator::CalibrateJoystick(const std::unique_ptr<MCU>& mcu) {
  float min_x = 0;
  float max_x = 0;
  float min_y = 0;
  float max_y = 0;

  uint64_t end_time = mcu->Millis() + 15000;  // 15 seconds from now.
  while (mcu->Millis() < end_time) {
    mcu->UpdateHallData();
    float z = mcu->GetHallZ();
    float x = mcu->GetHallX() / z;
    float y = mcu->GetHallY() / z;

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

  WriteToSerial(mcu, center_x);
  WriteToSerial(mcu, center_y);
  WriteToSerial(mcu, range);
}

void Configurator::SaveCalibration(const std::unique_ptr<MCU>& mcu) {
  int bytes_to_read = 12;
  int address = 0;
  while (address < bytes_to_read) {
    if (mcu->SerialAvailable()) {
      mcu->EEPROMUpdate(address, mcu->SerialRead());
      address++;
    }
  }
  mcu->SerialWrite(0);  // Done.
}

void Configurator::StoreProfiles(const std::unique_ptr<MCU>& mcu) {
  while (mcu->SerialAvailable() < 2) {}
  int num_bytes = mcu->SerialRead() << 8 | mcu->SerialRead();
  int base_address = 12;  // 0-11 are reserved for joystick calibration values.
  int curr_address = base_address;
  while (curr_address < base_address + num_bytes) {
    if (mcu->SerialAvailable()) {
      uint8_t data = mcu->SerialRead();
      mcu->EEPROMUpdate(curr_address, data);
      curr_address++;
    }
  }
  mcu->SerialWrite(0);  // Done.
}

void Configurator::Configure(std::unique_ptr<MCU> mcu) {
  while(true) {
    if (mcu->SerialAvailable() > 0) {
      uint8_t data = mcu->SerialRead();
      if (data > 4) {
        mcu->SerialWrite(1);  // Error.
        continue;
      }
      mcu->SerialWrite(0);  // OK.
      switch(data) {
      case 0:
        FetchStoredBounds(mcu);
        break;
      case 1:
        FetchJoystickCoords(mcu);
        break;
      case 2:
        CalibrateJoystick(mcu);
        break;
      case 3:
        SaveCalibration(mcu);
        break;
      case 4:
        StoreProfiles(mcu);
        break;
      }
    }
  }
}
