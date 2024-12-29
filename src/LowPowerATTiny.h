#pragma once

#include <avr/interrupt.h>
#include <avr/power.h>  // Power management
#include <avr/sleep.h>  // Sleep Modes
#include <avr/wdt.h>

#include "LowPowerCommon.h"

//#include <SoftwareSerial.h>
//SoftwareSerial Serial(2, 3);  // RX and TX

namespace low_power {

class ArduinoLowPowerATTiny;
ArduinoLowPowerATTiny *selfArduinoLowPowerATTiny = nullptr;

/**
 * @brief Low Power Management for ATTiny:
 * - deep sleep: can be woken up by pins or time based. Internally
 * we use the watchdog.
 *
 * @author Phil Schatzmann
 * see https://www.re-innovation.co.uk/docs/sleep-modes-on-attiny85/
 */

class ArduinoLowPowerATTiny : public ArduinoLowPowerCommon {
 public:
  ArduinoLowPowerATTiny() { selfArduinoLowPowerATTiny = this; }

  /// you cant do any processing which we sleep
  bool isProcessingOnSleep(sleep_mode_enum_t sleep_mode) { return false; }

  /// sets processor into sleep mode
  bool sleep(void) override {
    if (sleep_mode == sleep_mode_enum_t::deepSleep) {
      open_watchdog_cycle = 0;
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      if (sleep_time_us > 0) setupWatchdog();
      doDeepSleep();
      // for the case where we need to sleep for multiple cycles
      while (open_watchdog_cycle > 0) {
        doDeepSleep();
      }
      wdt_disable();
      set_sleep_mode(SLEEP_MODE_IDLE);
    } else {
      // disable all except the timer 1
      set_sleep_mode(SLEEP_MODE_IDLE);
      power_all_disable();
      power_timer0_enable();
      delay(sleep_time_us / 1000);
      power_all_enable();
    }
    return true;
  }

  bool setSleepTime(uint32_t time, time_unit_t time_unit_type) override {
    sleep_time_us = toUs(time, time_unit);
    return true;
  }

  bool addWakeupPin(int pin, pin_change_t change_type) override {
    // ADCSRA = 0;  // ADC disabled
    attachInterrupt(pin, pinWakupCB,
                    change_type == pin_change_t::on_high
                        ? RISING
                        : FALLING);  // interrupt on falling edge of pin2
    return true;
  }

  bool isModeSupported(sleep_mode_enum_t sleep_mode) override { return true; }

  void clear() {
    LP_LOG("clear");
    ArduinoLowPowerCommon::clear();
    sleep_time_us = 0;
    open_watchdog_cycle = 0;
    pin_mask = 0;
    set_sleep_mode(SLEEP_MODE_IDLE);
  }

  /// Called by watchdog interrupt
  static void processWatchdogCycle() {
    if (selfArduinoLowPowerATTiny != nullptr &&
        selfArduinoLowPowerATTiny->open_watchdog_cycle > 0) {
      selfArduinoLowPowerATTiny->open_watchdog_cycle--;
    }
  }

 protected:
  int open_watchdog_cycle = 0;
  uint32_t sleep_time_us = 0;
  uint32_t pin_mask = 0;
  uint16_t timings_ms[8] = {15, 30, 60, 120, 250, 500, 1000, 2000};


  void setupWatchdog() {
    int sleep_time_ms = sleep_time_us / 1000;
    int idx = getTimeIdx(sleep_time_ms);
    open_watchdog_cycle = sleep_time_ms / timings_ms[idx];
    wdt_enable(idx);  // Set WDog
  }

  // get the closest smaller time
  int getTimeIdx(int ms) {
    for (int j = 0; j < 5; j++) {
      if (timings_ms[j + 1] > ms) return j;
    }
    // 1sec
    return 6;
  }

  static void pinWakupCB() {}

  // set processor into deep sleep
  void doDeepSleep() {
    cli();
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();
    if (open_watchdog_cycle <= 0) {
      // after waking up
      sleep_disable();
    }
    sei();
  }
};

static ArduinoLowPowerATTiny LowPower;

}  // namespace low_power


// watchdog interrupt
ISR(WDT_vect) {
  wdt_reset();
  if (low_power::selfArduinoLowPowerATTiny != nullptr)
    low_power::selfArduinoLowPowerATTiny->processWatchdogCycle();

}  // end of WDT_vect
