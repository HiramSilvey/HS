// Copyright 2021 Hiram Silvey

#include <memory>

#include "controller.h"
#include "ns_controller.h"
#include "usb_controller.h"

std::unique_ptr<Controller> controller;

void setup() {
  // Order by priority.
  std::vector<std::unique_ptr<Controller>> controllers = {
    std::make_unique<USBController>(),
    std::make_unique<NSController>(),
  };
  while(true) {
    for (auto& c : controllers) {
      if (c->Init()) {
        controller = std::move(c);
        return;
      }
    }
    delay(50);
  }
}

void loop() {
  controller->Loop();
}
