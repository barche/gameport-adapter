// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "DigitalPin.h"
#include "HidJoystick.h"

#include "CHFlightstickPro.h"
#include "GenericJoystick.h"
#include "GrIP.h"
#include "Logitech.h"
#include "Sidewinder.h"
#include "ThrustMaster.h"

static Joystick *createJoystick() {
  return new Logitech;
}

void setup() {
    // DEBUG information: Debugging is turned off by default
    // Comment the "NDEBUG" line in "Utilities.h" to enable logging to the serial monitor
    initLog();
}

void loop() {

  static auto hidJoystick = []() {
      HidJoystick hidJoystick;
      hidJoystick.init(createJoystick());
      return hidJoystick;
  }();

  hidJoystick.update();
}
