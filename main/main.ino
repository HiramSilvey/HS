// Copyright 2021 Hiram Silvey

#include <memory>

#include "pins.h"
#include "configurator.h"
#include "controller.h"
#include "ns_controller.h"
#include "pc_controller.h"
#include "teensy.h"
#include "tlv493d_sensor.h"

std::unique_ptr<Controller> controller;
std::unique_ptr<Teensy> teensy;
std::unique_ptr<Tlv493d> sensor;
extern uint8_t nsgamepad_active;

void setup() {
  delay(100);

  teensy = std::make_unique<Teensy>();
  sensor = std::make_unique<Tlv493d>();
  if (teensy->DigitalReadLow(kRightIndexExtra)) {
    Configurator::Configure();
  }

  while(true) {
    if (nsgamepad_active && NSController::Active()) {
      controller = std::make_unique<NSController>(std::move(teensy),
                                                  std::move(sensor));
      break;
    } else if (PCController::Active()) {
      controller = std::make_unique<PCController>(std::move(teensy),
                                                  std::move(sensor));
      break;
    }
    delay(50);
  }
}

void loop() {
  controller->Loop();
}
