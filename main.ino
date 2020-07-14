#include <GamecubeAPI.h>

// Controller and console.
CGamecubeController controller(7); // Necessary; pin unused.
CGamecubeConsole console(8);       // Bi-directional data pin.

// Structured data to send to the console.
Gamecube_Data_t data = defaultGamecubeData;

// Default output data values.
const int kOff               = 0;
const int kOn                = 1;
const int kStickNeutral      = 128;
const int kStickMin          = 0;
const int kStickMax          = 255;
const int kTriggerLightPress = 74;

// Joystick pins; spaced to avoid crosstalk.
const int kJoystickX = A0;
const int kJoystickY = A7;

// C-stick pins.
const int kCstickUp    = 26;
const int kCstickDown  = 27;
const int kCstickLeft  = 28;
const int kCstickRight = 29;

// Trigger pins.
const int kL         = 32;
const int kRLight    = 33;

// Button pins.
const int kA     = 36;
const int kB     = 37;
const int kX     = 38;
const int kY     = 39;
const int kZ     = 40;
const int kStart = 41;

// D-pad pins.
const int kDUp = 44;
const int kDDown = 45;
const int kDLeft = 46;
const int kDRight = 47;

void setupDigitalPins() {
  pinMode(kCUp, INPUT_PULLUP);
  pinMode(kCDown, INPUT_PULLUP);
  pinMode(kCLeft, INPUT_PULLUP);
  pinMode(kCRight, INPUT_PULLUP);
  pinMode(kL, INPUT_PULLUP);
  pinMode(kRLight, INPUT_PULLUP);
  pinMode(kA, INPUT_PULLUP);
  pinMode(kB, INPUT_PULLUP);
  pinMode(kX, INPUT_PULLUP);
  pinMode(kY, INPUT_PULLUP);
  pinMode(kZ, INPUT_PULLUP);
  pinMode(kStart, INPUT_PULLUP);
  pinMode(kDUp, INPUT_PULLUP);
  pinMode(kDDown, INPUT_PULLUP);
  pinMode(kDLeft, INPUT_PULLUP);
  pinMode(kDRight, INPUT_PULLUP);
}

void setup() {
  setupDigitalPins();
  controller.read();
}

void loop() {
  // C-stick SOCD neutral.
  int c_x = kStickNeutral;
  if (digitalRead(kCstickLeft) == LOW && digitalRead(kCstickRight) == HIGH) c_x = kStickMin;
  if (digitalRead(kCstickRight) == LOW && digitalRead(kCstickLeft) == HIGH) c_x = kStickMax;
  int c_y = kStickNeutral;
  if (digitalRead(kCstickDown) == LOW && digitalRead(kCstickUp) == HIGH) c_y = kStickMin;
  if (digitalRead(kCstickUp) == LOW && digitalRead(kCstickDown) == HIGH) c_y = kStickMax;

  // Read & output data.
  data.report.xAxis = analogRead(kJoystickX) >> 2;
  data.report.yAxis = analogRead(kJoystickY) >> 2;
  data.report.cxAxis = c_x;
  data.report.cyAxis = c_y;
  data.report.l = kOn ? digitalRead(kL) == LOW : kOff;
  data.report.right = kTriggerLightPress ? digitalRead(kRLight) == LOW : kOff;
  data.report.a = kOn ? digitalRead(kA) == LOW : kOff;
  data.report.b = kOn ? digitalRead(kB) == LOW : kOff;
  data.report.x = kOn ? digitalRead(kX) == LOW : kOff;
  data.report.y = kOn ? digitalRead(kY) == LOW : kOff;
  data.report.z = kOn ? digitalRead(kZ) == LOW : kOff;
  data.report.start = kOn ? digitalRead(kStart) == LOW : kOff;
  data.report.dup = kOn ? digitalRead(kDUp) == LOW : kOff;
  data.report.ddown = kOn ? digitalRead(kDDown) == LOW : kOff;
  data.report.dleft = kOn ? digitalRead(kDLeft) == LOW : kOff;
  data.report.dright = kOn ? digitalRead(kDRight) == LOW : kOff;

  console.write(data);
}
