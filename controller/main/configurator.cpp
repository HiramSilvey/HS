// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include "Arduino.h"
#include <EEPROM.h>
#include <Tlv493d.h>

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
  Tlv493d sensor = Tlv493d();
  sensor.begin();
  sensor.setAccessMode(sensor.LOWPOWERMODE);
  sensor.disableTemp();

  Serial.write(0);  // Begin calibration.

  int min_x = 0;
  int max_x = 0;
  int min_y = 0;
  int max_y = 0;

  uint64_t end_time = millis() + 60000;  // 1 minute from now.
  while (millis() < end_time) {
    sensor.updateData();
    float z = sensor.getZ();
    int x = sensor.getX() / z * 1000000;
    int y = sensor.getY() / z * 1000000;

    if (x < min_x) {
      min_x = x;
    } else if (x < max_x) {
      max_x = x;
    }
    if (y < min_y) {
      min_y = y;
    } else if (y < max_y) {
      max_y = y;
    }
  }
  

  SaveToEEPROM(min_x, 0);
  SaveToEEPROM(max_x, 4);
  SaveToEEPROM(min_y, 8);
  SaveToEEPROM(max_y, 12);

  Serial.write(0);  // End calibration;
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
