// Copyright 2021 Hiram Silvey

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

class Controller {
public:
  // Initializations to be run once before the main loop. Returns true on
  // success, false otherwise.
  virtual bool Init() = 0;

  // Main loop to be run each tick.
  virtual void Loop() = 0;

private:
  // Simultaneous opposing cardinal direction resolution.
  virtual int ResolveSOCD(int low_direction, int high_direction) = 0;
};

#endif  // CONTROLLER_H_
