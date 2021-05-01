#include <Tlv493d.h>
#include <EEPROM.h>
#include <Wire.h>

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

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Tlv493d sensor = Tlv493d();
  sensor.begin();
  sensor.setAccessMode(sensor.FASTMODE);
  sensor.disableTemp();
  // Set I2C clock to recommended 1MHz for FASTMODE.
  Wire.setClock(1000000);

  int min_x = 0;
  int max_x = 0;
  int min_y = 0;
  int max_y = 0;

  Serial.println("Beginning calibration...");
  Serial.println("Roll joystick in full 360 degree circles for 30 seconds.");
  unsigned long start_time = millis();
  while (millis() - start_time < 30000) {
    sensor.updateData();
    int x = sensor.getX() * 1000;
    int y = sensor.getY() * 1000;
    min_x = x < min_x ? x : min_x;
    max_x = x > max_x ? x : max_x;
    min_y = y < min_y ? y : min_y;
    max_y = y > max_y ? y : max_y;
  }
  Serial.println("Measuring complete!\n");
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
