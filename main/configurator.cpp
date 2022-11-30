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

void FetchStoredBounds(const Teensy& teensy) {
  int center_x = util::GetIntFromEEPROM(teensy, 0);
  int center_y = util::GetIntFromEEPROM(teensy, 4);
  int range = util::GetIntFromEEPROM(teensy, 8);
  int16_t xy_angle_ticks = util::GetShortFromEEPROM(teensy, 12);
  int16_t xz_angle_ticks = util::GetShortFromEEPROM(teensy, 14);
  int16_t yz_angle_ticks = util::GetShortFromEEPROM(teensy, 16);

  WriteIntToSerial(teensy, center_x);
  WriteIntToSerial(teensy, center_y);
  WriteIntToSerial(teensy, range);
  WriteShortToSerial(teensy, xy_angle_ticks);
  WriteShortToSerial(teensy, xz_angle_ticks);
  WriteShortToSerial(teensy, yz_angle_ticks);
}

void FetchJoystickCoords(Teensy& teensy, std::unique_ptr<HallJoystick>& joystick) {
  int neutral_x = ReadIntFromSerial(teensy);
  int neutral_y = ReadIntFromSerial(teensy);
  int range = ReadIntFromSerial(teensy);
  uint16_t xy_angle_ticks = ReadShortFromSerial(teensy);
  uint16_t xz_angle_ticks = ReadShortFromSerial(teensy);
  uint16_t yz_angle_ticks = ReadShortFromSerial(teensy);

  joystick->set_x_in(neutral_x, range);
  joystick->set_y_in(neutral_y, range);
  joystick->set_xy_angle(xy_angle_ticks);
  joystick->set_xz_angle(xz_angle_ticks);
  joystick->set_yz_angle(yz_angle_ticks);

  HallJoystick::Coordinates coords = joystick->GetCoordinates(teensy);

  WriteIntToSerial(teensy, coords.x);
  WriteIntToSerial(teensy, coords.y);
}

void CalibrateJoystick(Teensy& teensy) {
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

  int center_x = (min_x + range_x) * 1000000;
  int center_y = (min_y + range_y) * 1000000;
  int range = range_x >= range_y ? range_x * 1000000 : range_y * 1000000;

  WriteIntToSerial(teensy, center_x);
  WriteIntToSerial(teensy, center_y);
  WriteIntToSerial(teensy, range);
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
	  internal::FetchStoredBounds(*teensy);
	  break;
	case 1:
	  internal::FetchJoystickCoords(*teensy, joystick);
	  break;
	case 2:
	  internal::CalibrateJoystick(*teensy);
	  break;
	case 3:
	  internal::SaveCalibration(*teensy);
	  break;
	case 4:
	  internal::StoreProfiles(*teensy);
	  break;
      }
    }
  }
}

}  // namespace configurator
}  // namespace hs
