// Copyright 2021 Hiram Silvey

#ifndef PINS_H_
#define PINS_H_

// Button pins.
const int kThumbTop = 0;
const int kThumbMiddle = 1;
const int kThumbBottom = 2;
const int kIndexTop = 3;
const int kIndexMiddle = 4;
const int kMiddleTop = 5;
const int kMiddleMiddle = 6;
const int kMiddleBottom = 7;
const int kRingTop = 8;
const int kRingMiddle = 9;
const int kRingBottom = 10;
const int kPinkyTop = 11;
const int kPinkyMiddle = 12;
const int kPinkyBottom = 13;
const int kLeftRingExtra = 14;
const int kLeftMiddleExtra = 15;
const int kLeftIndexExtra = 16;
const int kRightIndexExtra = 17;
const int kRightMiddleExtra = 20;
const int kRightRingExtra = 21;

// Joystick data and clock pins. Must be paired SDA and SCL pins on the
// microcontroller.
const int kJoystickSDA = 18;
const int kJoystickSCL = 19;

// Gamecube data pins.
const int kGameCubeController = 22;  // Necessary; pin unused.
const int kGameCubeConsole = 23;  // Bi-directional data pin; RJ45 brown stripe.

#endif  // PINS_H_
