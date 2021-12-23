// Copyright 2021 Hiram Silvey

#ifndef NSPAD_H_
#define NSPAD_H_

class NSPad {
public:
  virtual ~NSPad() {}

  virtual int DPadCentered() const = 0;
  virtual int DPadUp() const = 0;
  virtual int DPadDown() const = 0;
  virtual int DPadLeft() const = 0;
  virtual int DPadRight() const = 0;
  virtual int DPadUpLeft() const = 0;
  virtual int DPadUpRight() const = 0;
  virtual int DPadDownLeft() const = 0;
  virtual int DPadDownRight() const = 0;

  virtual void SetLeftXAxis(uint8_t val) const = 0;
  virtual void SetLeftYAxis(uint8_t val) const = 0;
  virtual void SetRightXAxis(uint8_t val) const = 0;
  virtual void SetRightYAxis(uint8_t val) const = 0;
  virtual void SetDPad(int8_t direction) const = 0;
  virtual void Press(uint8_t button_id) const = 0;
  virtual void ReleaseAll() const = 0;
  virtual void Loop() const = 0;
};

#endif  // NSPAD_H_
