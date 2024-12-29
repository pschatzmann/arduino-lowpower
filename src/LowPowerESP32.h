#pragma once

#include "LowPowerCommon.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp32-hal-touch.h"
#include "esp_wifi.h"

#define TOUCH_THREASHOLD 40

namespace low_power {

/// wakup type
enum class wakeup_t { ext0, ext1 };

#if defined(CONFIG_IDF_TARGET_ESP32C3) || \
    defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
#define ESP_STD_SLEEP 0
#else
#define ESP_STD_SLEEP 1

#endif

/**
 * @brief Low Power Management for ESP32:
 * - In Modem-sleep mode, ESP32 will close the Wi-Fi module circuit
 * between the two DTIM Beacon intervals in order to save power.
 * - In Light-sleep mode, the digital peripherals, most of the RAM, and CPUs are
 * clock-gated and their supply voltage is reduced.
 * - In Deep-sleep mode, the CPUs, most of the RAM, and all digital peripherals
 * that are clocked from APB_CLK are powered off.
 *
 * Additional ESP32 specific functionality/features:
 * - wakeup by touch pin
 * - supports multiple wakup sources
 * - supports modemSleep
 *
 * Attention: at wakup of deep sleep we restart in setup.
 *
 * @author Phil Schatzmann
 * for details see
 * -
 * https://docs.espressif.com/projects/arduino-esp32/en/latest/api/deepsleep.html
 *
 */

class ArduinoLowPowerESP32 : public ArduinoLowPowerCommon {
 public:
  bool isProcessingOnSleep(sleep_mode_enum_t sleep_mode) {
    bool result = false;
    switch (sleep_mode) {
      case sleep_mode_enum_t::noSleep:
        result = true;
        break;
      case sleep_mode_enum_t::lightSleep:
        result = false;
        break;
      case sleep_mode_enum_t::deepSleep:
        result = false;
        break;
      case sleep_mode_enum_t::modemSleep:
        result = true;
        break;
    }
    return result;
  }

  /// sets processor into sleep mode
  bool sleep(void) override {
    LP_LOG("sleep");
    switch (sleep_mode) {
      case sleep_mode_enum_t::lightSleep:
        LP_LOG("light sleep start");
        esp_light_sleep_start();
        LP_LOG("light sleep end");
        return true;
      case sleep_mode_enum_t::deepSleep:
        LP_LOG("deep sleep start");
        esp_deep_sleep_start();
        return true;
      case sleep_mode_enum_t::noSleep:
        wifiSetPS(WIFI_PS_NONE);
        delay(sleep_time_us / 1000);
        return true;

      case sleep_mode_enum_t::modemSleep:
        wifiSetPS(WIFI_PS_MAX_MODEM);
        delay(sleep_time_us / 1000);
        //    wifiSetPS(WIFI_PS_MIN_MODEM);
        return true;
    }
    return false;
  }

  bool setSleepTime(uint32_t time, time_unit_t time_unit) override {
    if (sleep_mode == sleep_mode_enum_t::modemSleep) return false;
    sleep_time_us = toUs(time, time_unit);
    return esp_sleep_enable_timer_wakeup(toUs(time, time_unit)) == ESP_OK;
  }

#if ESP_STD_SLEEP
  bool addWakeupPin(int pin, pin_change_t change_type) override {
    if (sleep_mode == sleep_mode_enum_t::modemSleep) return false;
    pin_mask |= 1 << pin;
    switch (change_type) {
      case pin_change_t::on_high: {
        if (wakeup_type == wakeup_t::ext0)
          esp_sleep_enable_ext0_wakeup((gpio_num_t)pin, 1);
        else
          esp_sleep_enable_ext1_wakeup_io(pin_mask,
                                          ESP_EXT1_WAKEUP_ANY_HIGH);

        // gpio is tied to GND in order to wake up in HIGH
        rtc_gpio_pulldown_en((gpio_num_t)pin);
        // Disable PULL_UP in order to allow it to wakeup on HIGH
        rtc_gpio_pullup_dis((gpio_num_t)pin);
      } break;

      case pin_change_t::on_low: {
        if (wakeup_type == wakeup_t::ext0)
          esp_sleep_enable_ext0_wakeup((gpio_num_t)pin, 0);
        else
          esp_sleep_enable_ext1_wakeup_io(pin_mask,
                                          ESP_EXT1_WAKEUP_ALL_LOW);

        rtc_gpio_pullup_en((gpio_num_t)pin);
        rtc_gpio_pulldown_dis((gpio_num_t)pin);
      } break;

      default:
        // unsupported change type
        return false;
    }
    return true;
  }
#endif

#if CONFIG_IDF_TARGET_ESP32H2
  bool addWakeupPin(int pin, pin_change_t change_type) override {
    if (sleep_mode == sleep_mode_enum_t::modemSleep) return false;

    gpio_wakeup_enable((gpio_num_t)pin, change_type == pin_change_t::on_high
                                            ? GPIO_INTR_POSEDGE
                                            : GPIO_INTR_NEGEDGE);
    esp_sleep_enable_gpio_wakeup();
    return true;
  }
#endif

#if !ESP_STD_SLEEP && !CONFIG_IDF_TARGET_ESP32H2
  bool addWakeupPin(int pin, pin_change_t change_type) override {
    if (sleep_mode == sleep_mode_enum_t::modemSleep) return false;
    pin_mask |= 1 << pin;
    if (sleep_mode == sleep_mode_enum_t::deepSleep) {
      esp_deep_sleep_enable_gpio_wakeup(pin_mask,
                                        change_type == pin_change_t::on_high
                                            ? ESP_GPIO_WAKEUP_GPIO_HIGH
                                            : ESP_GPIO_WAKEUP_GPIO_LOW);
      return true;
    }
    if (sleep_mode == sleep_mode_enum_t::lightSleep) {
      gpio_wakeup_enable((gpio_num_t)pin, change_type == pin_change_t::on_high
                                              ? GPIO_INTR_POSEDGE
                                              : GPIO_INTR_NEGEDGE);
      esp_sleep_enable_gpio_wakeup();
      return true;
    }
    return false;
  }
#endif

  /// Wakup by touch pin
  bool addWakeupTouchPin(int pin, int touch_threshold = TOUCH_THREASHOLD) {
#if ESP_STD_SLEEP
    if (!isTouchPin(pin)) return false;
    touchSleepWakeUpEnable(pin, touch_threshold);
    return true;
#else
    return false;
#endif
  }

  /**
    @brief There are two types for ESP32, ext0 and ext1 .
    ext0 uses RTC_IO to wakeup thus requires RTC peripherals
    to be on while ext1 uses RTC Controller so does not need
    peripherals to be powered on.
    Note that using internal pullups/pulldowns also requires
    RTC peripherals to be turned on.
  */
  void setWakeupType(wakeup_t wakeup) { wakeup_type = wakeup; }

  bool isModeSupported(sleep_mode_enum_t sleep_mode) override { return true; }

  /// Reset to the initial state
  void clear() override {
    ArduinoLowPowerCommon::clear();
    touch_pins.clear();
    wifiSetPS(WIFI_PS_NONE);
    sleep_time_us = 0;
    pin_mask = 0;
  }

  void setCpuFrequencyMhz(int mhz){
    ::setCpuFrequencyMhz(mhz);
  }

 protected:
  wakeup_t wakeup_type = wakeup_t::ext1;
  uint32_t sleep_time_us = 0;
  std::vector<int> touch_pins;
  uint32_t pin_mask = 0;

  void wifiSetPS(wifi_ps_type_t type) {
#if !CONFIG_IDF_TARGET_ESP32H2
    esp_wifi_set_ps(type);
#endif
  }

  bool isTouchPin(int pin) {
    for (int tp : touch_pins) {
      if (tp == pin) return true;
    }
    return false;
  }
};

RTC_DATA_ATTR static ArduinoLowPowerESP32 LowPower;

}  // namespace low_power