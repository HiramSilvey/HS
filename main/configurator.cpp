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

  uint64_t end_time = millis() + 15000;  // 15 seconds from now.
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

  float range_x = (max_x - min_x) / 2.0;
  float range_y = (max_y - min_y) / 2.0;

  int center_x = (min_x + range_x) * 1000000;
  int center_y = (min_y + range_y) * 1000000;
  int range = range_x >= range_y ? range_x * 1000000 : range_y * 1000000;

  Serial.write(0);  // Done.

  WriteToSerial(center_x);
  WriteToSerial(center_y);
  WriteToSerial(range);

  while(true) {
    if (Serial.available() > 0) {
      byte cmd = Serial.read();
      switch(cmd) {
      case 0:
        Serial.write(0);
        sensor.updateData();
        float z = sensor.getZ();
        int x = sensor.getX() / z * 1000000;
        int y = sensor.getY() / z * 1000000;
        WriteToSerial(x);
        WriteToSerial(y);
        break;
      case 1:
        Serial.write(0);
        int bytes_to_read = 12;
        int address = 0;
        while (address < bytes_to_read) {
          if (Serial.available()) {
            EEPROM.update(address, Serial.read());
            address++;
          }
        }
        Serial.write(0);
        return;
      }
    }
  }
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
