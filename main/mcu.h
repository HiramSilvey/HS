// Copyright 2021 Hiram Silvey

#ifndef MCU_H_
#define MCU_H_

class MCU {
public:
  virtual int digitalRead(uint8_t pin) = 0;
}

#endif  // MCU_H_
