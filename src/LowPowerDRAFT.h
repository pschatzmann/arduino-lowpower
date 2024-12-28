#pragma once

#include "LowPowerCommon.h"

namespace low_power {

/**
 * @brief Low Power Management for TBD.
 * @author Phil Schatzmann
 *
 */

class ArduinoLowPowerTemplate : public ArduinoLowPowerCommon {
 public:

  bool isProcessingOnSleep(sleep_mode_enum_t sleep_mode) {
    bool result = false;
    switch (sleep_mode) {
      case sleep_mode_enum_t::modemSleep:
        result = true;
        break;
      case sleep_mode_enum_t::noSleep:
        result = true;
        break;
      case sleep_mode_enum_t::lightSleep:
        result = false;
        break;
      case sleep_mode_enum_t::deepSleep:
        result = false;
        break;
    }
    return result;
  }

  /// sets processor into sleep mode
  bool sleep(void) override {
    bool rc = false;
    switch (sleep_mode) {
      // In Modem-sleep mode, ESP8266 will close the Wi-Fi module circuit
      // between the two DTIM Beacon intervals in order to save power
      case sleep_mode_enum_t::modemSleep:
        break;

      // wakup by external pin
      case sleep_mode_enum_t::lightSleep:
        break;

      // responsible for periodic wake-ups
      case sleep_mode_enum_t::deepSleep: {
      } break;

      case sleep_mode_enum_t::noSleep:
        break;
    }

    return rc;
  }

  bool setSleepTime(uint32_t time, time_unit_t time_unit_type) override {
    return true;
  }

  bool addWakeupPin(int pin, pin_change_t change_type) override {
    return true;
  }

  bool isModeSupported(sleep_mode_enum_t sleep_mode) override {
    return true;
  }

  void clear() {
    ArduinoLowPowerCommon::clear();
    sleep_time_us = 0;
    sleep_option = 1;
    is_instant = false;
    gpio_count = 0;
  }

 protected:
};

static ArduinoLowPowerTemplate LowPower;

}  // namespace low_power