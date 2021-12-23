// Copyright 2021 Hiram Silvey

#ifndef TEENSY_H_
#define TEENSY_H_

#include "mcu.h"

#include <EEPROM.h>
#include <Tlv493d.h>

#include "Arduino.h"

class Teensy : public MCU {
public:
  Teensy() {
    sensor_.begin();
    sensor_.setAccessMode(sensor_.LOWPOWERMODE);
    sensor_.disableTemp();
  }
  inline bool DigitalReadLow(uint8_t pin) const override {
    return digitalRead(pin) == LOW;
  }

  inline int Constrain(int amount, int low, int high) const override {
    return constrain(amount, low, high);
  }

  inline void SerialWrite(uint8_t val) const override {
    Serial.write(val);
  }
  inline void SerialWrite(uint8_t* vals, int size) const override {
    Serial.write(vals, size);
  }
  inline int SerialRead() const override {
    return Serial.read();
  }
  inline int SerialAvailable() const override {
    return Serial.available();
  }

  inline unsigned long Millis() const override {
    return millis();
  }

  inline void JoystickUseManualSend() const override {
    Joystick.useManualSend(true);
  }
  inline void SetJoystickX(int val) const override {
    Joystick.X(val);
  }
  inline void SetJoystickY(int val) const override {
    Joystick.Y(val);
  }
  inline void SetJoystickZ(int val) const override {
    Joystick.Z(val);
  }
  inline void SetJoystickZRotate(int val) const override {
    Joystick.Zrotate(val);
  }
  inline void SetJoystickSliderLeft(int val) const override {
    Joystick.sliderLeft(val);
  }
  inline void SetJoystickSliderRight(int val) const override {
    Joystick.sliderRight(val);
  }
  inline void SetJoystickButton(uint8_t pin, bool active) const override {
    Joystick.button(pin, active);
  }
  inline void SetJoystickHat(int angle) const override {
    Joystick.hat(angle);
  }
  inline void JoystickSendNow() const override {
    Joystick.send_now();
  }

  inline uint8_t EEPROMRead(int addr) const override {
    return EEPROM.read(addr);
  }
  inline void EEPROMUpdate(int addr, uint8_t val) const override {
    EEPROM.update(addr, val);
  }

  inline void UpdateHallData() override {
    sensor_.updateData();
  }
  inline float GetHallX() override {
    return sensor_.getX();
  }
  inline float GetHallY() override {
    return sensor_.getY();
  }
  inline float GetHallZ() override {
    return sensor_.getZ();
  }
private:
  Tlv493d sensor_;
};

#endif  // TEENSY_H_
