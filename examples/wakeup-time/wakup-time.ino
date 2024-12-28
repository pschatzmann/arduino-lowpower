/**
 * @brief Example which sets a defined sleep and active time.
 * @author Phil Schatzmann
 */
#include "LowPower.h"

void setup() {
  Serial.begin(115200);

  // setup low power definition
  LowPower.setSleepMode(sleep_mode_enum_t::deepSleep);
  LowPower.setSleepTime(6000, time_unit_t::ms);
  LowPower.setActiveTime(2000, time_unit_t::ms);
}

void loop() {
  Serial.print("loop running for 2 sec");
  LowPower.process();
}