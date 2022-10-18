// Copyright 2021 Hiram Silvey

#include <memory>

#include "configurator.h"
#include "controller.h"
#include "ns_controller.h"
#include "nspad_impl.h"
#include "pc_controller.h"
#include "pins.h"
#include "teensy_impl.h"

std::unique_ptr<hs::Controller> controller;
extern uint8_t nsgamepad_active;
extern volatile uint8_t usb_configuration;

void InitPins() {
  pinMode(hs::pins::kThumbTop, INPUT_PULLUP);
  pinMode(hs::pins::kThumbMiddle, INPUT_PULLUP);
  pinMode(hs::pins::kThumbBottom, INPUT_PULLUP);
  pinMode(hs::pins::kIndexTop, INPUT_PULLUP);
  pinMode(hs::pins::kIndexMiddle, INPUT_PULLUP);
  pinMode(hs::pins::kMiddleTop, INPUT_PULLUP);
  pinMode(hs::pins::kMiddleMiddle, INPUT_PULLUP);
  pinMode(hs::pins::kMiddleBottom, INPUT_PULLUP);
  pinMode(hs::pins::kRingTop, INPUT_PULLUP);
  pinMode(hs::pins::kRingMiddle, INPUT_PULLUP);
  pinMode(hs::pins::kRingBottom, INPUT_PULLUP);
  pinMode(hs::pins::kPinkyTop, INPUT_PULLUP);
  pinMode(hs::pins::kPinkyMiddle, INPUT_PULLUP);
  pinMode(hs::pins::kPinkyBottom, INPUT_PULLUP);
  pinMode(hs::pins::kLeftOuter, INPUT_PULLUP);
  pinMode(hs::pins::kLeftInner, INPUT_PULLUP);
  delayMicroseconds(50);  // Allow the resistors time to pull up the pins fully.
}

void setup() {
  InitPins();

  auto teensy = std::make_unique<hs::TeensyImpl>();
  if (teensy->DigitalReadLow(hs::pins::kLeftOuter)) {
    hs::configurator::Configure(std::move(teensy));
    exit(0);
  }

  while (true) {
    if (usb_configuration) {
      if (nsgamepad_active) {
	auto nspad = std::make_unique<hs::NSPadImpl>();
	controller = std::make_unique<hs::NSController>(std::move(teensy),
							std::move(nspad));
      } else {
	controller = std::make_unique<hs::PCController>(std::move(teensy));
      }
      break;
    }
    delay(50);
  }
}

void loop() { controller->Loop(); }
