/**
 * @brief Example which sets a active time after waking up:
 * The wakup is triggered by the gpio pin 1 changing to high.
 * @author Phil Schatzmann
 */

#include "LowPower.h"

void setup() {
  Serial.begin(115200);

  // setup low power definition
  LowPower.setSleepMode(sleep_mode_enum_t::deepSleep);
  LowPower.addWakeupPin(1, pin_change_t::on_high);
  LowPower.setActiveTime(2000, time_unit_t::ms); // comment out to run once
}

void loop() {
  Serial.print("loop running for 2 sec");
  LowPower.process();
}