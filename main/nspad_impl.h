// Copyright 2021 Hiram Silvey

#ifndef NSPAD_IMPL_H_
#define NSPAD_IMPL_H_

#include "nspad.h"

#include "usb_nsgamepad.h"

class NSPadImpl : public NSPad {
public:
  inline int DPadCentered() const override {
    return NSGAMEPAD_DPAD_CENTERED;
  }
  inline int DPadUp() const override {
    return NSGAMEPAD_DPAD_UP;
  }
  inline int DPadDown() const override {
    return NSGAMEPAD_DPAD_DOWN;
  }
  inline int DPadLeft() const override {
    return NSGAMEPAD_DPAD_LEFT;
  }
  inline int DPadRight() const override {
    return NSGAMEPAD_DPAD_RIGHT;
  }
  inline int DPadUpLeft() const override {
    return NSGAMEPAD_DPAD_UP_LEFT;
  }
  inline int DPadUpRight() const override {
    return NSGAMEPAD_DPAD_UP_RIGHT;
  }
  inline int DPadDownLeft() const override {
    return NSGAMEPAD_DPAD_DOWN_LEFT;
  }
  inline int DPadDownRight() const override {
    return NSGAMEPAD_DPAD_DOWN_RIGHT;
  }

  inline void SetLeftXAxis(uint8_t val) const override {
    NSGamepad.leftXAxis(val);
  }
  inline void SetLeftYAxis(uint8_t val) const override {
    NSGamepad.leftYAxis(val);
  }
  inline void SetRightXAxis(uint8_t val) const override {
    NSGamepad.rightXAxis(val);
  }
  inline void SetRightYAxis(uint8_t val) const override {
    NSGamepad.rightYAxis(val);
  }
  inline void SetDPad(int8_t direction) const override {
    NSGamepad.dPad(direction);
  }
  inline void Press(uint8_t button_id) const override {
    NSGamepad.press(button_id);
  }
  inline void ReleaseAll() const override {
    NSGamepad.releaseAll();
  }
  inline void Loop() const override {
    NSGamepad.loop();
  }
};

#endif  // NSPAD_IMPL_H_
