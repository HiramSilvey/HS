// Copyright 2024 Hiram Silvey

#ifndef TEENSY_H_
#define TEENSY_H_

namespace hs {

class Teensy {
 public:
  virtual ~Teensy() {}

  // Arduino
  virtual bool DigitalReadLow(uint8_t pin) const = 0;
  virtual void Exit(int status) const = 0;

  // Arduino: Math
  virtual int Constrain(int amount, int low, int high) const = 0;

  // Arduino: Serial
  virtual void SerialWrite(uint8_t val) const = 0;
  virtual void SerialWrite(uint8_t* vals, int size) const = 0;
  virtual int SerialRead() const = 0;
  virtual int SerialAvailable() const = 0;

  // Arduino: Time
  virtual unsigned long Millis() const = 0;
  virtual unsigned long Micros() const = 0;

  // Arduino: Joystick
  virtual void JoystickUseManualSend() const = 0;
  virtual void SetJoystickX(int val) const = 0;
  virtual void SetJoystickY(int val) const = 0;
  virtual void SetJoystickZ(int val) const = 0;
  virtual void SetJoystickZRotate(int val) const = 0;
  virtual void SetJoystickSliderLeft(int val) const = 0;
  virtual void SetJoystickSliderRight(int val) const = 0;
  virtual void SetJoystickButton(uint8_t pin, bool active) const = 0;
  virtual void SetJoystickHat(int angle) const = 0;
  virtual void JoystickSendNow() const = 0;

  // EEPROM
  virtual uint8_t EEPROMRead(int addr) const = 0;
  virtual void EEPROMUpdate(int addr, uint8_t val) const = 0;

  // Tlv493d
  virtual void UpdateHallData() = 0;
  virtual float GetHallX() = 0;
  virtual float GetHallY() = 0;
  virtual float GetHallZ() = 0;
};

}  // namespace hs

#endif  // TEENSY_H_
