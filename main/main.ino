// Copyright 2021 Hiram Silvey

#include "controller.h"
#include "ngc_controller.h"
#include "usb_controller.h"

Controller controller;
USBController usb_controller = USBController();
NGCController ngc_controller = NGCController();

bool InitController() {
  if (ngc_controller.Init()) {
    controller = ngc_controller;
    return true;
  }
  if (usb_controller.Init()) {
    controller = usb_controller;
    return true;
  }
  return false;
}

void setup() {
  while(!InitController()) {
    delay(50);
  }
}

void loop() {
  controller.Loop();
}
