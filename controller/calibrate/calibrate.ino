#include <vector>
#include <numeric>
#include <string>

#include <Tlv493d.h>
#include <EEPROM.h>
#include <Wire.h>

Tlv493d sensor = Tlv493d();

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

int GetAverageY(bool down) {
  Serial.print("Hold joystick all the way to the ");
  if (down) {
    Serial.print("down");
  } else {
    Serial.print("up");
  }
  Serial.println(" and rotate the shaft for 15 seconds.");
  delay(5000);
  std::vector<double> points;
  for (int i = 0; i < 1000; i++) {
    sensor.updateData();
    float z = sensor.getZ();
    points.push_back(sensor.getY() / z);
    delay(10);
  }
  if (down) {
    Serial.print("Down");
  } else {
    Serial.print("Up");
  }
  Serial.println(" calibration complete!");
  if (points.size() > 0) {
    return std::accumulate(points.begin(), points.end(), 0.0) / points.size() * 1000000;
  }
  return 0;
}

int GetAverageX(bool left) {
  Serial.print("Hold joystick all the way to the ");
  if (left) {
    Serial.print("left");
  } else {
    Serial.print("right");
  }
  Serial.println(" and rotate the shaft for 15 seconds.");
  delay(5000);
  std::vector<double> points;
  for (int i = 0; i < 1000; i++) {
    sensor.updateData();
    float z = sensor.getZ();
    points.push_back(sensor.getX() / z);
    delay(10);
  }
  if (left) {
    Serial.print("Left");
  } else {
    Serial.print("Right");
  }
  Serial.println(" calibration complete!");
  if (points.size() > 0) {
    return std::accumulate(points.begin(), points.end(), 0.0) / points.size() * 1000000;
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  sensor.begin();
  sensor.setAccessMode(sensor.LOWPOWERMODE);
  sensor.disableTemp();

  Serial.println("Beginning calibration...");
  int min_x = GetAverageX(true);
  int max_x = GetAverageX(false);
  int min_y = GetAverageY(true);
  int max_y = GetAverageY(false);

  Serial.print("Lowest X = ");
  Serial.print(min_x);
  Serial.print(", Highest X = ");
  Serial.print(max_x);
  Serial.println();
  Serial.print("Lowest Y = ");
  Serial.print(min_y);
  Serial.print(", Highest Y = ");
  Serial.print(max_y);
  Serial.println("\n");

  Serial.println("Writing data to EEPROM...");

  SaveToEEPROM(min_x, 0);
  SaveToEEPROM(max_x, 4);
  SaveToEEPROM(min_y, 8);
  SaveToEEPROM(max_y, 12);

  Serial.print("Done! Calibration complete!");
  exit(0);
}

void loop() {
}
