// Copyright 2021 Hiram Silvey

#ifndef MCU_H_
#define MCU_H_

class MCU {
public:
  virtual ~MCU() {}
  virtual bool DigitalReadLow(uint8_t pin) const = 0;
};

#endif  // MCU_H_
