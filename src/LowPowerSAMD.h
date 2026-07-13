#pragma once

#include "LowPowerCommon.h"
#include "drivers/samd/samd.h"

namespace low_power {

/**
 * @brief Low Power Management for SAMD.
 * Depends on RTCZero!
 * @author Phil Schatzmann
 *
 */

class ArduinoLowPowerSAMD : public ArduinoLowPowerCommon {
 public:
  bool setSleepMode(sleep_mode_enum_t sleep_mode) {
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
      case sleep_mode_enum_t::lightSleep:
        samd.sleep(sleep_time_us / 1000);
        rc = true;
        break;

      case sleep_mode_enum_t::deepSleep:
        samd.deepSleep(sleep_time_us / 1000);
        rc = true;
        break;

      case sleep_mode_enum_t::modemSleep:
      case sleep_mode_enum_t::noSleep:
        delay(sleep_time_us / 1000);
        rc = true;
        break;
    }

    return rc;
  }

  bool setSleepTime(uint32_t time, time_unit_t time_unit_type) override {
    sleep_time_us = (toUs(time, time_unit_type));
    return true;
  }

  bool addWakeupPin(int pin, pin_change_t change_type) override {
    samd.attachInterruptWakeup(pin, callback, toMode(change_type));
    return true;
  }

  void clear() {
    ArduinoLowPowerCommon::clear();
    samd.detachAdcInterrupt();
    sleep_time_us = 0;
  }
  
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
 protected:
  uint32_t sleep_time_us = 0;
  ArduinoLowPowerClass samd;

  static void callback() {}

  PinStatus toMode(pin_change_t ct) {
    switch (ct) {
      case pin_change_t::on_high:
        return RISING;
      case pin_change_t::on_low:
        return FALLING;
      default:
        return CHANGE;
    }
  }
};

static ArduinoLowPowerSAMD LowPower;

}  // namespace low_power