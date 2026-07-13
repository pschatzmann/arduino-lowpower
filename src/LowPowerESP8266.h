#pragma once

#include <ESP8266WiFi.h>
#include <PolledTimeout.h>
#include <coredecls.h>          // crc32()
#include <include/WiFiState.h>  // WiFiState structure details
#include "LowPowerCommon.h"
#include "gpio.h"

namespace low_power {

/**
 * @brief Low Power Management for ESP8266.
 * - In Modem-sleep mode, ESP8266 will close the Wi-Fi module circuit
 * between the two DTIM Beacon intervals in order to save power
 * - During Light-sleep, the CPU is suspended and will not respond to the
 * signals and interrupts from the peripheral hardware interfaces. Therefore,
 * ESP8266 needs to be woken up via external GPIO.
 * - During Deep-sleep the chip will turn off Wi-Fi connectivity and data
 * connection; only the RTC module is still working, responsible for periodic
 * wake-ups. To enable Deep-sleep, you need to connect GPIO16 to the EXT_RSTB
 * pin of ESP8266.
 * @author Phil Schatzmann
 * for details see
 * -
 * https://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf
 *
 */

class ArduinoLowPowerESP8266 : public ArduinoLowPowerCommon {
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
        rc = wifi_set_sleep_type(MODEM_SLEEP_T);
        break;

      // wakup by external pin
      case sleep_mode_enum_t::lightSleep:
        if (sleep_time_us != 0 || gpio_count == 0) return false;
        rc = wifi_set_sleep_type(LIGHT_SLEEP_T);
        break;

      // responsible for periodic wake-ups
      case sleep_mode_enum_t::deepSleep: {
        if (gpio_count != 0 || sleep_time_us == 0) return false;
        system_deep_sleep_set_option(sleep_option);
        if (is_instant)
          system_deep_sleep(sleep_time_us);
        else
          system_deep_sleep_instant(sleep_time_us);

        rc = true;
      } break;

      case sleep_mode_enum_t::noSleep:
        rc = true;
        break;
    }

    return rc;
  }

  bool setSleepTime(uint32_t time, time_unit_t time_unit_type) override {
    if (sleep_mode == sleep_mode_enum_t::modemSleep) return false;
    setSleepMode(sleep_mode_enum_t::deepSleep);
    sleep_time_us = (toUs(time, time_unit_type));
    return gpio_count == 0;
  }

  bool addWakeupPin(int pin, pin_change_t change_type) override {
    if (sleep_mode == sleep_mode_enum_t::modemSleep) return false;
    setSleepMode(sleep_mode_enum_t::lightSleep);
    GPIO_INT_TYPE int_type = (change_type == pin_change_t::on_high)
                                 ? GPIO_PIN_INTR_HILEVEL
                                 : GPIO_PIN_INTR_LOLEVEL;
    gpio_pin_wakeup_enable(digitalPinToInterrupt(pin), int_type);
    gpio_count++;
    return sleep_time_us == 0;
  }

  /**
   * @brief Deep sleep options: values from 0 to 4
   * 1: RF calibration: Power consumption is high
   * 2: No RF calibration after waking up: Power consumption is low
   * 3: No RF: Power consumption is lowest
   */
  void setDeepSleepOption(uint8_t option) { sleep_option = option; }

  /// if instant == true -> instantly deep sleep w/o delay
  void setInstant(bool instant) { is_instant = instant; }

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
  uint64_t sleep_time_us = 0;
  uint8_t sleep_option = 1;  // 1 (rf calibration)
  bool is_instant = false;
  uint16_t gpio_count = 0;
};

static ArduinoLowPowerESP8266 LowPower;

}  // namespace low_power