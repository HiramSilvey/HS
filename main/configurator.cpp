// Copyright 2021 Hiram Silvey

#include "configurator.h"

#include <memory>

#include "hall_joystick.h"
#include "math.h"
#include "teensy.h"
#include "util.h"

namespace hs {

namespace {

void WriteToSerial(const Teensy& teensy, int val) {
  uint8_t* bytes = (uint8_t*)&val;
  teensy.SerialWrite(bytes, 4);
}

void WriteToSerial(const Teensy& teensy, double val) {
  uint8_t* bytes = (uint8_t*)&val;
  teensy.SerialWrite(bytes, 8);
}

void WriteErr(const Teensy& teensy) { teensy.SerialWrite(1); }
void WriteOk(const Teensy& teensy) { teensy.SerialWrite(0); }

}  // namespace

Configurator::Configurator(std::unique_ptr<Teensy> teensy)
    : teensy_(std::move(teensy)),
      joystick_(std::make_unique<HallJoystick>(*teensy_, /*min=*/0,
					       /*max=*/1023, /*threshold=*/0)),
      neutral_x_(util::ReadIntFromEEPROM(*teensy_, 0)),
      neutral_y_(util::ReadIntFromEEPROM(*teensy_, 4)),
      range_(util::ReadIntFromEEPROM(*teensy_, 8)),
      range_tick_(range_ / 512),
      xy_angle_ticks_(util::ReadShortFromEEPROM(*teensy_, 12)),
      xz_angle_ticks_(util::ReadShortFromEEPROM(*teensy_, 14)),
      yz_angle_ticks_(util::ReadShortFromEEPROM(*teensy_, 16)) {}

void Configurator::IncNeutralX() {
  neutral_x_ += range_tick_;
  joystick_->set_x_in(neutral_x_, range_);
  WriteOk(*teensy_);
}
void Configurator::DecNeutralX() {
  neutral_x_ -= range_tick_;
  joystick_->set_x_in(neutral_x_, range_);
  WriteOk(*teensy_);
}
void Configurator::IncNeutralY() {
  neutral_y_ += range_tick_;
  joystick_->set_y_in(neutral_y_, range_);
  WriteOk(*teensy_);
}
void Configurator::DecNeutralY() {
  neutral_y_ -= range_tick_;
  joystick_->set_y_in(neutral_y_, range_);
  WriteOk(*teensy_);
}
void Configurator::IncRange() {
  range_ += range_tick_;
  joystick_->set_x_in(neutral_x_, range_);
  joystick_->set_y_in(neutral_y_, range_);
  WriteOk(*teensy_);
}
void Configurator::DecRange() {
  range_ -= range_tick_;
  joystick_->set_x_in(neutral_x_, range_);
  joystick_->set_y_in(neutral_y_, range_);
  WriteOk(*teensy_);
}
void Configurator::IncXYAngle() {
  joystick_->set_xy_angle(++xy_angle_ticks_);
  WriteOk(*teensy_);
}
void Configurator::DecXYAngle() {
  joystick_->set_xy_angle(--xy_angle_ticks_);
  WriteOk(*teensy_);
}
void Configurator::IncXZAngle() {
  joystick_->set_xz_angle(++xz_angle_ticks_);
  WriteOk(*teensy_);
}
void Configurator::DecXZAngle() {
  joystick_->set_xz_angle(--xz_angle_ticks_);
  WriteOk(*teensy_);
}
void Configurator::IncYZAngle() {
  joystick_->set_yz_angle(++yz_angle_ticks_);
  WriteOk(*teensy_);
}
void Configurator::DecYZAngle() {
  joystick_->set_yz_angle(--yz_angle_ticks_);
  WriteOk(*teensy_);
}

void Configurator::FetchJoystickCoords() {
  HallJoystick::Coordinates coords = joystick_->GetCoordinates(*teensy_);

  WriteToSerial(*teensy_, coords.x);
  WriteToSerial(*teensy_, coords.y);
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

  WriteOk(*teensy_);
}

void Configurator::SaveCalibration() {
  util::WriteIntToEEPROM(*teensy_, 0, neutral_x_);
  util::WriteIntToEEPROM(*teensy_, 4, neutral_y_);
  util::WriteIntToEEPROM(*teensy_, 8, range_);
  util::WriteShortToEEPROM(*teensy_, 12, xy_angle_ticks_);
  util::WriteShortToEEPROM(*teensy_, 14, xz_angle_ticks_);
  util::WriteShortToEEPROM(*teensy_, 16, yz_angle_ticks_);

  range_tick_ = range_ / 512;
  WriteOk(*teensy_);
}

void Configurator::StoreProfiles() {
  while (teensy_->SerialAvailable() < 2) {
  }
  int num_bytes = teensy_->SerialRead() << 8 | teensy_->SerialRead();
  int base_address = 18;  // 0-17 are reserved for joystick calibration values.
  int curr_address = base_address;
  while (curr_address < base_address + num_bytes) {
    if (teensy_->SerialAvailable()) {
      uint8_t data = teensy_->SerialRead();
      teensy_->EEPROMUpdate(curr_address, data);
      curr_address++;
    }
  }
  WriteOk(*teensy_);
}

void Configurator::FetchJoystickState() {
  HallJoystick::Bounds x_in = joystick_->x_in();
  HallJoystick::Bounds y_in = joystick_->y_in();

  WriteToSerial(*teensy_, x_in.min);
  WriteToSerial(*teensy_, x_in.max);
  WriteToSerial(*teensy_, y_in.min);
  WriteToSerial(*teensy_, y_in.max);
  WriteToSerial(*teensy_, joystick_->xy_angle());
  WriteToSerial(*teensy_, joystick_->xz_angle());
  WriteToSerial(*teensy_, joystick_->yz_angle());
  WriteToSerial(*teensy_, joystick_->out_min());
  WriteToSerial(*teensy_, joystick_->out_max());
  WriteToSerial(*teensy_, joystick_->out_neutral());
}

void Configurator::Loop() {
  while (true) {
    if (teensy_->SerialAvailable() > 0) {
      uint8_t data = teensy_->SerialRead();
      if (data > 16) {
	WriteErr(*teensy_);
	continue;
      }
      WriteOk(*teensy_);
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
	  IncNeutralX();
	  break;
	case 5:
	  DecNeutralX();
	  break;
	case 6:
	  IncNeutralY();
	  break;
	case 7:
	  DecNeutralY();
	  break;
	case 8:
	  IncRange();
	  break;
	case 9:
	  DecRange();
	  break;
	case 10:
	  IncXYAngle();
	  break;
	case 11:
	  DecXYAngle();
	  break;
	case 12:
	  IncXZAngle();
	  break;
	case 13:
	  DecXZAngle();
	  break;
	case 14:
	  IncYZAngle();
	  break;
	case 15:
	  DecYZAngle();
	  break;
	case 16:
	  FetchJoystickState();
	  break;
      }
    }
  }
}

}  // namespace hs
