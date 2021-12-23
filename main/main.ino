// Copyright 2021 Hiram Silvey

#include <memory>

#include "pins.h"
#include "configurator.h"
#include "controller.h"
#include "ns_controller.h"
#include "pc_controller.h"
#include "teensy_impl.h"
#include "nspad_impl.h"

std::unique_ptr<Controller> controller;
extern uint8_t nsgamepad_active;

void setup() {
  delay(100);

  auto teensy = std::make_unique<TeensyImpl>();
  if (teensy->DigitalReadLow(kRightIndexExtra)) {
    Configurator::Configure(std::move(teensy));
    exit(0);
  }

  while(true) {
    if (nsgamepad_active && NSController::Active()) {
      auto nsgamepad = std::make_unique<NSPadImpl>();
      controller = std::make_unique<NSController>(std::move(teensy), std::move(nsgamepad));
      break;
    } else if (PCController::Active()) {
      controller = std::make_unique<PCController>(std::move(teensy));
      break;
    }
    delay(50);
  }
}

void loop() {
  controller->Loop();
}
