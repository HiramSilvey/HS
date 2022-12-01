// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include <memory>

#include "hall_joystick.h"
#include "math.h"
#include "teensy.h"
#include "util.h"

namespace hs {
namespace configurator {

namespace {

int ReadIntFromSerial(const Teensy& teensy) {
  while (teensy.SerialAvailable() < 4) {}
  return teensy.SerialRead() << 24 | teensy.SerialRead() << 16
    | teensy.SerialRead() << 8 | teensy.SerialRead();
}

uint16_t ReadShortFromSerial(const Teensy& teensy) {
  while (teensy.SerialAvailable() < 2) {}
  return teensy.SerialRead() << 8 | teensy.SerialRead();
}

void WriteIntToSerial(const Teensy& teensy, int val) {
  uint8_t bytes[4] = {
      static_cast<uint8_t>(val >> 24), static_cast<uint8_t>(val >> 16 & 0xFF),
      static_cast<uint8_t>(val >> 8 & 0xFF), static_cast<uint8_t>(val & 0xFF)};
  teensy.SerialWrite(bytes, 4);
}

void WriteShortToSerial(const Teensy& teensy, int16_t val) {
  uint8_t bytes[2] = {static_cast<uint8_t>(val >> 8 & 0xFF),
		      static_cast<uint8_t>(val & 0xFF)};
  teensy.SerialWrite(bytes, 2);
}

}  // namespace

namespace internal {

const double kTickAngle = HallJoystick::GetAngleFromTicks(/*ticks=*/1);

void SetXYIn(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  int range = ReadIntFromSerial(teensy);
  joystick->set_x_in(ReadIntFromSerial(teensy), range);
  joystick->set_y_in(ReadIntFromSerial(teensy), range);
  teensy.SerialWrite(0);  // Done.
}

void IncXYAngle(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  joystick->set_xy_angle(joystick->xy_angle() + kTickAngle);
  teensy.SerialWrite(0);  // Done.
}

void SetXZAngle(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  joystick->set_xz_angle(ReadShortFromSerial(teensy));
  teensy.SerialWrite(0);  // Done.
}

void SetYZAngle(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  joystick->set_yz_angle(ReadShortFromSerial(teensy));
  teensy.SerialWrite(0);  // Done.
}

void FetchJoystickCoords(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  HallJoystick::Coordinates coords = joystick->GetCoordinates(teensy);

  WriteIntToSerial(teensy, coords.x);
  WriteIntToSerial(teensy, coords.y);
}

void CalibrateJoystick(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  float min_x = 0;
  float max_x = 0;
  float min_y = 0;
  float max_y = 0;

  uint64_t end_time = teensy.Millis() + 15000;  // 15 seconds from now.
  while (teensy.Millis() < end_time) {
    teensy.UpdateHallData();
    float z = teensy.GetHallZ();
    float x = teensy.GetHallX() / z;
    float y = teensy.GetHallY() / z;

    if (x < min_x) {
      min_x = x;
    } else if (x > max_x) {
      max_x = x;
    }
    if (y < min_y) {
      min_y = y;
    } else if (y > max_y) {
      max_y = y;
    }
  }

  float range_x = (max_x - min_x) / 2.0;
  float range_y = (max_y - min_y) / 2.0;

  int neutral_x = (min_x + range_x) * 1000000;
  int neutral_y = (min_y + range_y) * 1000000;
  int range = range_x >= range_y ? range_x * 1000000 : range_y * 1000000;

  joystick->set_x_in(neutral_x, range);
  joystick->set_y_in(neutral_y, range);
}

void SaveCalibration(const Teensy& teensy) {
  int bytes_to_read = 18;
  int address = 0;
  while (address < bytes_to_read) {
    if (teensy.SerialAvailable()) {
      teensy.EEPROMUpdate(address, teensy.SerialRead());
      address++;
    }
  }
  teensy.SerialWrite(0);  // Done.
}

void StoreProfiles(const Teensy& teensy) {
  while (teensy.SerialAvailable() < 2) {}
  int num_bytes = teensy.SerialRead() << 8 | teensy.SerialRead();
  int base_address = 18;  // 0-17 are reserved for joystick calibration values.
  int curr_address = base_address;
  while (curr_address < base_address + num_bytes) {
    if (teensy.SerialAvailable()) {
      uint8_t data = teensy.SerialRead();
      teensy.EEPROMUpdate(curr_address, data);
      curr_address++;
    }
  }
  teensy.SerialWrite(0);  // Done.
}

}  // namespace internal

void Configure(std::unique_ptr<Teensy> teensy) {
  auto joystick = std::make_unique<HallJoystick>(*teensy, 0, 1023,
						 /*threshold=*/{0, 0});
  while (true) {
    if (teensy->SerialAvailable() > 0) {
      uint8_t data = teensy->SerialRead();
      if (data > 4) {
	teensy->SerialWrite(1);  // Error.
	continue;
      }
      teensy->SerialWrite(0);  // OK.
      switch (data) {
	case 0:
	  internal::FetchJoystickCoords(*teensy, joystick);
	  break;
	case 1:
	  internal::CalibrateJoystick(*teensy, joystick);
	  break;
	case 2:
	  internal::SaveCalibration(*teensy);
	  break;
	case 3:
	  internal::StoreProfiles(*teensy);
	  break;
	case 4:
	  internal::SetXYIn(*teensy, joystick);
	  break;
	case 5:
	  internal::SetXYAngle(*teensy, joystick);
	  break;
	case 6:
	  internal::SetXZAngle(*teensy, joystick);
	  break;
	case 7:
	  internal::SetYZAngle(*teensy, joystick);
	  break;
      }
    }
  }
}

}  // namespace configurator
}  // namespace hs
