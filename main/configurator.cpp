// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include "Arduino.h"
#include <EEPROM.h>
#include <Tlv493d.h>

void Configurator::StoreProfiles() {
  Serial.write(0);  // OK.
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

void Configurator::CalibrateJoystick() {
  Serial.write(0);  // OK.

  Tlv493d sensor = Tlv493d();
  sensor.begin();
  sensor.setAccessMode(sensor.LOWPOWERMODE);
  sensor.disableTemp();

  float min_x = 0;
  float max_x = 0;
  float min_y = 0;
  float max_y = 0;

  uint64_t end_time = millis() + 60000;  // 1 minute from now.
  while (millis() < end_time) {
    sensor.updateData();
    float z = sensor.getZ();
    float x = sensor.getX() / z;
    float y = sensor.getY() / z;

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

  int x_diff = (max_x - min_x) * 100000;
  int y_diff = (max_y - min_y) * 100000;

  SaveToEEPROM(min_x*1000000+x_diff, 0);
  SaveToEEPROM(max_x*1000000-x_diff, 4);
  SaveToEEPROM(min_y*1000000+y_diff, 8);
  SaveToEEPROM(max_y*1000000-y_diff, 12);

  Serial.write(0);  // Done.
}

void Configurator::Configure() {
  while(true) {
    if (Serial.available() > 0) {
      byte data = Serial.read();
      switch(data) {
      case 0:
        return;
      case 1:
        StoreProfiles();
        break;
      case 2:
        CalibrateJoystick();
        break;
      }
    }
  }
}
