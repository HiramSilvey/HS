// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include <memory>

#include "hall_joystick.h"
#include "math.h"
#include "teensy.h"
#include "util.h"

namespace hs {

namespace {

uint16_t ReadShortFromSerial(const Teensy& teensy) {
  while (teensy.SerialAvailable() < 2) {
  }
  return teensy.SerialRead() << 8 | teensy.SerialRead();
}

int ReadIntFromSerial(const Teensy& teensy) {
  while (teensy.SerialAvailable() < 4) {
  }
  return teensy.SerialRead() << 24 | teensy.SerialRead() << 16 |
	 teensy.SerialRead() << 8 | teensy.SerialRead();
}

void WriteShortToSerial(const Teensy& teensy, int16_t val) {
  uint8_t bytes[2] = {static_cast<uint8_t>(val >> 8 & 0xFF),
		      static_cast<uint8_t>(val & 0xFF)};
  teensy.SerialWrite(bytes, 2);
}

void WriteIntToSerial(const Teensy& teensy, int val) {
  uint8_t bytes[4] = {
      static_cast<uint8_t>(val >> 24), static_cast<uint8_t>(val >> 16 & 0xFF),
      static_cast<uint8_t>(val >> 8 & 0xFF), static_cast<uint8_t>(val & 0xFF)};
  teensy.SerialWrite(bytes, 4);
}

}  // namespace

void Configurator::WriteOk() { teensy_->SerialWrite(0); }

void Configurator::IncRange() { joystick_->set_x_in(neutral_x_, ++range); }

void SetXYIn(const Teensy& teensy,
	     const std::unique_ptr<HallJoystick>& joystick) {
  int range = ReadIntFromSerial(teensy);
  joystick->set_x_in(ReadIntFromSerial(teensy), range);
  joystick->set_y_in(ReadIntFromSerial(teensy), range);
  WriteOk(teensy);
}

void Configurator::IncXYAngle() {
  joystick_->set_xy_angle(++xy_angle_ticks_);
  WriteOk();
}
void Configurator::DecXYAngle() {
  joystick_->set_xy_angle(--xy_angle_ticks_);
  WriteOk();
}
void Configurator::IncXZAngle() {
  joystick_->set_xz_angle(++xz_angle_ticks_);
  WriteOk();
}
void Configurator::DecXZAngle() {
  joystick_->set_xz_angle(--xy_angle_ticks_);
  WriteOk();
}
void Configurator::IncYZAngle() {
  joystick_->set_yz_angle(++yz_angle_ticks_);
  WriteOk();
}
void Configurator::DecYZAngle() {
  joystick_->set_yz_angle(--yz_angle_ticks_);
  WriteOk();
}

void FetchJoystickCoords(const Teensy& teensy,
			 const std::unique_ptr<HallJoystick>& joystick) {
  HallJoystick::Coordinates coords = joystick->GetCoordinates(teensy);

  WriteIntToSerial(teensy, coords.x);
  WriteIntToSerial(teensy, coords.y);
}

void Configurator::CalibrateJoystick() {
  float min_x = 0;
  float max_x = 0;
  float min_y = 0;
  float max_y = 0;

  uint64_t end_time = teensy_->Millis() + 15000;  // 15 seconds from now.
  while (teensy_->Millis() < end_time) {
    teensy_->UpdateHallData();
    float z = teensy_->GetHallZ();
    float x = teensy_->GetHallX() / z;
    float y = teensy_->GetHallY() / z;

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

  neutral_x_ = (min_x + range_x) * 1000000;
  neutral_y_ = (min_y + range_y) * 1000000;
  range_ = range_x >= range_y ? range_x * 1000000 : range_y * 1000000;
  range_tick_ = range_ / 512;
  xy_angle_ticks_ = 0;
  xz_angle_ticks_ = 0;
  yz_angle_ticks_ = 0;

  joystick_->set_x_in(neutral_x_, range_);
  joystick_->set_y_in(neutral_y_, range_);
  joystick_->set_xy_angle(xy_angle_ticks_);
  joystick_->set_xz_angle(xz_angle_ticks_);
  joystick_->set_yz_angle(yz_angle_ticks_);
}

void SaveCalibration(const Teensy& teensy,
		     const std::unique_ptr<HallJoystick>& joystick) {
  const HallJoystick::Bounds x_in = joystick->x_in();
  const HallJoystick::Bounds y_in = joystick->y_in();
  int range = (x_in.max - x_in.min) / 2;
  int neutral_x = x_in.min + range;
  int neutral_y = y_in.min + range;
  util::WriteIntToEEPROM(teensy, 0, neutral_x);
  util::WriteIntToEEPROM(teensy, 4, neutral_y);
  util::WriteIntToEEPROM(teensy, 8, range);
  util::WriteShortToEEPROM(teensy, joystick->xy_angle(), neutral_x);
  WriteOk(teensy);
}

void StoreProfiles(const Teensy& teensy) {
  while (teensy.SerialAvailable() < 2) {
  }
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
  WriteOk(teensy);
}

Configurator::Configurator(std::unique_ptr<Teensy> teensy)
    : teensy_(std::move(teensy)),
      joystick_(std::make_unique<HallJoystick>(*teensy_, /*min=*/0,
					       /*max=*/1023, /*threshold=*/0)),
      neutral_x_(util::GetIntFromEEPROM(*teensy_, 0)),
      neutral_y_(util::GetIntFromEEPROM(*teensy_, 4)),
      range_(util::GetIntFromEEPROM(*teensy_, 8)),
      range_tick_(range_ / 512),
      xy_angle_ticks_(util::GetShortFromEEPROM(*teensy_, 12)),
      xz_angle_ticks_(util::GetShortFromEEPROM(*teensy_, 14)),
      yz_angle_ticks_(util::GetShortFromEEPROM(*teensy_, 16)) {}

void Configurator::Loop() {
  while (true) {
    if (teensy_->SerialAvailable() > 0) {
      uint8_t data = teensy_->SerialRead();
      if (data > 4) {
	teensy_->SerialWrite(1);  // Error.
	continue;
      }
      teensy_->SerialWrite(0);  // OK.
      switch (data) {
	case 0:
	  FetchJoystickCoords();
	  break;
	case 1:
	  CalibrateJoystick();
	  break;
	case 2:
	  SaveCalibration();
	  break;
	case 3:
	  StoreProfiles();
	  break;
	case 4:
	  SetXYIn();
	  break;
	case 5:
	  IncXYAngle();
	  break;
	case 6:
	  DecXYAngle();
	  break;
	case 7:
	  IncXZAngle();
	  break;
	case 8:
	  DecXZAngle();
	  break;
	case 9:
	  IncYZAngle();
	  break;
	case 10:
	  DecYZAngle();
	  break;
      }
    }
  }
}

}  // namespace hs
}  // namespace hs
