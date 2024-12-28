#pragma once

#include <Arduino.h>

#include "LowPowerCommon.h"

namespace low_power {

enum class sleep_mode_enum_t {
  noSleep,
  lightSleep,
  deepSleep,
  modemSleep,
};

enum class time_unit_t {
  sec,
  ms,
  us,
};

enum class pin_change_t {
  on_high,
  on_low,
};

/**
 * @brief Common API for power saving modes for different processor
 * architectures
 * @author Phil Schatzmann
 */

class ArduinoLowPowerCommon {
 public:
  /// Sets processor into sleep mode
  virtual bool sleep(void) = 0;

  /// Defines the sleep time
  virtual bool setSleepTime(uint32_t time, time_unit_t time_unit_type) = 0;

  /// Defines the wakup pin
  virtual bool addWakeupPin(int pin, pin_change_t change_type) = 0;

  /// sets mc into sleep mode to sleep for indicated millis
  virtual bool sleepFor(uint32_t time, time_unit_t time_unit_type) {
    setSleepTime(time, time_unit_type);
    sleep();
    return true;
  }

  /// sets the flag to be active
  virtual void setActive(bool flag) { is_active = false; }

  /// Defiles the active time
  virtual void setActiveTime(uint32_t time, time_unit_t time_unit_type) {
    uint32_t timeout_us = toUs(time, time_unit_type);
    timeout_end = millis() + timeout_us / 1000;
    ;
  }

  /// Checks if we are active (not sleeping)
  virtual bool isActive() {
    if (timeout_end > 0 && timeout_end > millis()) return false;
    if (!is_active) return false;
    return true;
  }

  /// same as isActive()
  virtual operator bool() { return isActive(); }

  /// Defines the sleep mode
  virtual bool setSleepMode(sleep_mode_enum_t mode) {
    sleep_mode = mode;
    bool result = isModeSupported(mode);
    if (result && mode == sleep_mode_enum_t::modemSleep) sleep();
    return result;
  }

  /// @brief Triggers the processing to be active or sleeping based on the set
  /// definitions
  virtual void process() {
    if (isActive()) return;
    sleep();
    // recalculate next timeout
    setActiveTime(timeout_us, time_unit_t::us);
  }

  /// Returns true if processing is possible in the current sleep mode
  virtual bool isProcessingOnSleep(sleep_mode_enum_t sleep_mode) = 0;

  /// Returns true if processing is possible in the current sleep mode
  virtual bool isProcessingOnSleep() {
    return isProcessingOnSleep(sleep_mode);
  };

  /// Provides information if the indicated mode is supported 
  virtual bool isModeSupported(sleep_mode_enum_t sleep_mode) {
    bool result = true;
    switch (sleep_mode) {
      case sleep_mode_enum_t::noSleep:
        result = false;
        break;
      case sleep_mode_enum_t::lightSleep:
        result = true;
        break;
      case sleep_mode_enum_t::deepSleep:
        result = true;
        break;
      case sleep_mode_enum_t::modemSleep:
        result = true;
        break;
    }
    return result;
  }

  /// reset the processing
  virtual void clear() {
    sleep_mode = sleep_mode_enum_t::deepSleep;
    time_unit = time_unit_t::ms;
    timeout_us = 0;
    timeout_end = 0;
    is_active = true;
  }

 protected:
  bool is_active = true;
  uint32_t timeout_end = 0;
  uint32_t timeout_us = 0;
  time_unit_t time_unit = time_unit_t::ms;
  sleep_mode_enum_t sleep_mode = sleep_mode_enum_t::deepSleep;

  uint64_t toUs(uint64_t time, time_unit_t time_unit) {
    switch (time_unit) {
      case time_unit_t::sec:
        return time * 1000000;
      case time_unit_t::ms:
        return time * 1000;
      case time_unit_t::us:
        return time;
    }
    return 0;
  }
};

}  // namespace low_power