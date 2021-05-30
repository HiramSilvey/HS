// Copyright 2021 Hiram Silvey

#include <memory>

#include "controller.h"
#include "usb_controller.h"

std::unique_ptr<Controller> controller;

void setup() {
  // Ensure button states are settled before relying on them in initialization.
  delay(100);

  auto usb_controller = std::make_unique<USBController>();
  while(true) {
    if (usb_controller->Init()) {
      controller = std::move(usb_controller);
      return;
    }
    delay(50);
  }
}

void loop() {
  controller->Loop();
}
