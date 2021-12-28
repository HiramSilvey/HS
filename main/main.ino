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

void setup() {
  delay(100);

  auto teensy = std::make_unique<hs::TeensyImpl>();
  if (teensy->DigitalReadLow(hs::pins::kRightIndexExtra)) {
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
