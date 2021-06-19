#include "controller.h"

int Controller::ResolveSOCD(int low_direction, int high_direction, int joystick_min,
                            int joystick_max, int joystick_neutral) {
  if (!low_direction && !high_direction) {
    return joystick_neutral;
  }
  if (low_direction && !high_direction) {
    return digitalRead(low_direction) == LOW ? joystick_min : joystick_neutral;
  }
  if (!low_direction && high_direction) {
    return digitalRead(high_direction) == LOW ? joystick_max : joystick_neutral;
  }
  if (digitalRead(low_direction) == LOW
      && digitalRead(high_direction) == HIGH) return joystick_min;
  if (digitalRead(high_direction) == LOW
      && digitalRead(low_direction) == HIGH) return joystick_max;
  return joystick_neutral;
}
