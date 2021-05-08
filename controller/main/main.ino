// Copyright 2021 Hiram Silvey

#include <memory>

#include "controller.h"
#include "configurator.h"
#include "usb_controller.h"

std::unique_ptr<Controller> controller;

void setup() {
  auto usb_controller = std::make_unique<USBController>();
  while(true) {
    if (usb_controller->Init()) {
      if (digitalRead(kConfigureButton) == LOW) {
        Configure();
      }
      controller = std::move(usb_controller);
      return;
    }
    delay(50);
  }
}

void loop() {
  controller->Loop();
}
