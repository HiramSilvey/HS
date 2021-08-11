// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include "Arduino.h"
#include <EEPROM.h>
#include <Tlv493d.h>

void WriteToSerial(int val) {
  byte bytes[4] = {val >> 24,
                   val >> 16 & 0xFF,
                   val >> 8 & 0xFF,
                   val & 0xFF};
  Serial.write(bytes, 4);
}

void SaveToEEPROM(int val, int address) {
  byte one = val >> 24;
  byte two = val >> 16 & 0xFF;
  byte three = val >> 8 & 0xFF;
  byte four = val & 0xFF;

  EEPROM.update(address, one);
  EEPROM.update(address+1, two);
  EEPROM.update(address+2, three);
  EEPROM.update(address+3, four);
}

Configurator::Configurator() {
  sensor_ = Tlv493d();
  sensor_.begin();
  sensor_.setAccessMode(sensor_.LOWPOWERMODE);
  sensor_.disableTemp();
}

void Configurator::FetchJoystickCoords() {
  sensor_.updateData();
  float z = sensor_.getZ();
  int x = sensor_.getX() / z * 1000000;
  int y = sensor_.getY() / z * 1000000;

  WriteToSerial(x);
  WriteToSerial(y);
}

void Configurator::CalibrateJoystick() {
  float min_x = 0;
  float max_x = 0;
  float min_y = 0;
  float max_y = 0;

  uint64_t end_time = millis() + 15000;  // 15 seconds from now.
  while (millis() < end_time) {
    sensor_.updateData();
    float z = sensor_.getZ();
    float x = sensor_.getX() / z;
    float y = sensor_.getY() / z;

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

  WriteToSerial(center_x);
  WriteToSerial(center_y);
  WriteToSerial(range);
}

void Configurator::SaveCalibration() {
  int bytes_to_read = 12;
  int address = 0;
  while (address < bytes_to_read) {
    if (Serial.available()) {
      EEPROM.update(address, Serial.read());
      address++;
    }
  }
  Serial.write(0);  // Done.
}

void Configurator::StoreProfiles() {
  while (Serial.available() < 2) {}
  int num_bytes = Serial.read() << 8 | Serial.read();
  int base_address = 16;  // 0-15 are reserved for joystick calibration values.
  int curr_address = base_address;
  while (curr_address < base_address + num_bytes) {
    if (Serial.available()) {
      byte data = Serial.read();
      EEPROM.update(curr_address, data);
      curr_address++;
    }
  }
  Serial.write(0);  // Done.
}

void Configurator::Configure() {
  while(true) {
    if (Serial.available() > 0) {
      byte data = Serial.read();
      if (data > 3) {
        Serial.write(1);  // Error.
        continue;
      }
      Serial.write(0);  // OK.
      switch(data) {
      case 0:
        FetchJoystickCoords();
        break;
      case 1:
        CalibrateJoystick();
        break;
      case 2:
        SaveCalibration();
        break;
      case 3:
        StoreProfiles();
        break;
      }
    }
  }
}
