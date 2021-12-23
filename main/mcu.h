// Copyright 2021 Hiram Silvey

#ifndef MCU_H_
#define MCU_H_

class MCU {
public:
  virtual ~MCU() {}
  virtual int Constrain(int amount, int low, int high) const = 0;
  virtual bool DigitalReadLow(uint8_t pin) const = 0;
  virtual uint8_t EEPROMRead(int addr) const = 0;
};

#endif  // MCU_H_
