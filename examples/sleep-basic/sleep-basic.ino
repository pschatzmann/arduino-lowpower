/**
 * @brief We directly call sleepFor to trgger the sleeping for the
 * indicated time.
 * @author Phil Schatzmann
 */

#include "LowPower.h"

void setup() {
  Serial.begin(115200);
  // setup low power definition
  LowPower.setSleepMode(sleep_mode_enum_t::deepSleep); // or lightSleep
}

void loop() {
  Serial.print("loop every 1000 time_unit_t::ms");
  LowPower.sleepFor(1000, time_unit_t::ms);
}