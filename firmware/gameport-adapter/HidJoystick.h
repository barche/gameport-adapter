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

#pragma once

#include "Buffer.h"
#include "HidDevice.h"
#include "Joystick.h"
#include "Utilities.h"
#include <Arduino.h>

class HidJoystick {
public:
  bool init(Joystick *joystick) {
    if (!joystick || !joystick->init()) {
      return false;
    }

    m_joystick = joystick;
    m_hidDescription = createDescriptionGamePad(*m_joystick);
    m_subDescriptor = new HIDSubDescriptor{m_hidDescription.data, m_hidDescription.size};
    m_hidDevice.AppendDescriptor(m_subDescriptor);

    log("Detected device: %s", joystick->getDescription().name);
    return true;
  }

  bool update() {
    if (!m_joystick || !m_joystick->update()) {
      return false;
    }

    const auto packet = createPacketGamePad(*m_joystick);
    m_hidDevice.SendReport(DEVICE_ID, packet.data, packet.size);
    return true;
  }

private:
  using BufferType = Buffer<256>;
  static const uint8_t DEVICE_ID{3};

  static BufferType createDescription(const Joystick &joystick) {

    enum class ID : uint8_t {
      application = 0x01,
      button = 0x09,
      collection = 0xa1,
      end_collection = 0xc0,
      generic_desktop = 0x01,
      hat_switch = 0x39,
      input = 0x81,
      input_const = 0x03,
      input_data = 0x02,
      joystick = 0x04,
      logical_max = 0x26,
      logical_min = 0x15,
      report_count = 0x95,
      report_id = 0x85,
      report_size = 0x75,
      simulation_controls = 0x02,
      throttle = 0xbb,
      usage = 0x09,
      usage_max = 0x29,
      usage_min = 0x19,
      usage_page = 0x05,
    };

    const auto &desc = joystick.getDescription();
    BufferType buffer;
    auto filler = BufferFiller(buffer);

    auto pushData = [&filler](uint8_t size, uint8_t count) -> uint32_t {
      filler.push(ID::report_size).push(size);
      filler.push(ID::report_count).push(count);
      filler.push(ID::input).push(ID::input_data);
      return size * count;
    };

    filler.push(ID::usage_page).push(ID::generic_desktop);
    filler.push(ID::usage).push(ID::joystick);
    filler.push(ID::collection).push(ID::application);
    filler.push(ID::report_id).push(uint8_t(DEVICE_ID));

    uint32_t padding = 0u;

    // Push axes
    if (desc.numAxes > 0) {
      filler.push(ID::usage_page).push(ID::generic_desktop);
      for (auto i = 0u; i < desc.numAxes; i++) {
        static const uint8_t x_axis = 0x30;
        filler.push(ID::usage).push(uint8_t(x_axis + i));
      }
      filler.push(ID::logical_min).push(uint8_t(0));
      filler.push(ID::logical_max).push(uint16_t(1023));
      padding += pushData(10, desc.numAxes);
    }

    // Push hats
    if (desc.numHats > 0) {
      filler.push(ID::usage_page).push(ID::generic_desktop);
      filler.push(ID::usage).push(ID::hat_switch);
      filler.push(ID::logical_min).push(uint8_t(1));
      filler.push(ID::logical_max).push(uint16_t(8));
      padding += pushData(4, desc.numHats);
    }

    // Push buttons
    if (desc.numButtons > 0) {
      filler.push(ID::usage_page).push(ID::button);
      filler.push(ID::usage_min).push(uint8_t(1));
      filler.push(ID::usage_max).push(uint8_t(desc.numButtons));
      filler.push(ID::logical_min).push(uint8_t(0));
      filler.push(ID::logical_max).push(uint16_t(1));
      padding += pushData(1, desc.numButtons);
    }

    // Push padding
    static const auto bitsPerByte = 8u;
    padding %= bitsPerByte;
    if (padding) {
      filler.push(ID::report_size).push(uint8_t(bitsPerByte - padding));
      filler.push(ID::report_count).push(uint8_t(1));
      filler.push(ID::input).push(ID::input_const);
    }

    filler.push(ID::end_collection);
    return buffer;
  }

static BufferType createDescriptionGamePad(const Joystick &joystick) {

    enum class ID : uint8_t {
      physical = 0x00,
      application = 0x01,
      button = 0x09,
      collection = 0xa1,
      end_collection = 0xc0,
      generic_desktop = 0x01,
      hat_switch = 0x39,
      input = 0x81,
      input_const = 0x03,
      input_data = 0x02,
      gamepad = 0x05,
      logical_max = 0x26,
      logical_min = 0x15,
      report_count = 0x95,
      report_id = 0x85,
      report_size = 0x75,
      simulation_controls = 0x02,
      throttle = 0xbb,
      usage = 0x09,
      usage_max = 0x29,
      usage_min = 0x19,
      usage_page = 0x05,
      usage_x1 = 0x30,
      usage_y1 = 0x31,
      usage_x2 = 0x33,
      usage_y2 = 0x34,
    };

    const auto &desc = joystick.getDescription();
    BufferType buffer;
    auto filler = BufferFiller(buffer);

    filler.push(ID::usage_page).push(ID::generic_desktop);
    filler.push(ID::usage).push(ID::gamepad);
    filler.push(ID::collection).push(ID::application);
    filler.push(ID::collection).push(ID::physical);
    filler.push(ID::report_id).push(uint8_t(DEVICE_ID));

    filler.push(ID::usage_page).push(ID::button);
    filler.push(ID::usage_min).push(uint8_t(1));
    filler.push(ID::usage_max).push(uint8_t(16));
    filler.push(ID::logical_min).push(uint8_t(0));
    filler.push(ID::logical_max).push(uint16_t(1));
    filler.push(ID::report_size).push(uint8_t(1));
    filler.push(ID::report_count).push(uint8_t(16));

    filler.push(ID::input).push(ID::input_data);
    filler.push(ID::usage_page).push(ID::generic_desktop);
    filler.push(ID::usage).push(ID::usage_x1);
    filler.push(ID::usage).push(ID::usage_y1);
    filler.push(ID::usage).push(ID::usage_x2);
    filler.push(ID::usage).push(ID::usage_y2);
    filler.push(ID::logical_min).push(uint8_t(0));
    filler.push(ID::logical_max).push(uint16_t(1023));
    filler.push(ID::report_size).push(uint8_t(10));
    filler.push(ID::report_count).push(uint8_t(4));
    filler.push(ID::input).push(ID::input_data);

    filler.push(ID::end_collection);
    filler.push(ID::end_collection);
    return buffer;
  }

  static BufferType createPacket(const Joystick &joystick) {

    const auto &state = joystick.getState();
    const auto &description = joystick.getDescription();
    BufferType buffer;
    auto filler = BufferFiller(buffer);

    for (auto i = 0u; i < description.numAxes; i++) {
      filler.push(state.axes[i], 10);
    }

    for (auto i = 0u; i < description.numHats; i++) {
      filler.push(state.hats[i], 4);
    }

    if (description.numButtons) {
      filler.push(state.buttons, description.numButtons);
    }

    filler.allign();
    return buffer;
  }

  static uint16_t hatx(uint8_t h) {
    if(h == 2 || h == 3 || h == 4) {
      return 1023;
    }
    if(h == 6 || h == 7 || h == 8) {
      return 0;
    }
    return 512;
  }

  static uint16_t haty(uint8_t h) {
    if(h == 8 || h == 1 || h == 2) {
      return 0;
    }
    if(h == 6 || h == 5 || h == 4) {
      return 1023;
    }
    return 512;
  }

  static BufferType createPacketGamePad(const Joystick &joystick) {
    const auto &state = joystick.getState();
    BufferType buffer;
    auto filler = BufferFiller(buffer);
    uint16_t buttons = state.buttons;
    
    uint8_t h1 = state.hats[0];
    uint8_t h2 = state.hats[1];
    uint8_t h3 = state.hats[2];

    if(hatx(h2) > 512) bitSet(buttons, 9);
    if(haty(h2) > 512) bitSet(buttons, 10);
    if(hatx(h2) < 512) bitSet(buttons, 11);
    if(haty(h2) < 512) bitSet(buttons, 12);

    if(hatx(h3) > 512) bitSet(buttons, 13);
    if(haty(h3) > 512) bitSet(buttons, 14);
    if(hatx(h3) < 512) bitSet(buttons, 15);

    filler.push(buttons, 16);

    // axes
    uint16_t x2 = hatx(h1);
    uint16_t y2 = state.axes[2]; // haty(h1);
    filler.push(x2, 10);
    filler.push(y2, 10);
    filler.push(state.axes[0], 10);
    filler.push(state.axes[1], 10);

    filler.allign();
    return buffer;
  }

  Joystick *m_joystick{};
  BufferType m_hidDescription{};
  HIDSubDescriptor *m_subDescriptor{};
  HidDevice m_hidDevice;
};

