// Copyright 2021 Hiram Silvey

#include "main/joystick.h"

#include <Tlv493d.h>

void Joystick::Setup() {
  sensor_.begin();
}

int Joystick::GetX() {
  // TODO(hiram): implement
  return 0;
}

int Joystick::GetY() {
  // TODO(hiram): implement
  return 0;
}
