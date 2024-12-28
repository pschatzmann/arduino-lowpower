/**
 * @brief Example which triggers sleep w/o wakeup
 * @author Phil Schatzmann
 */

#include "LowPower.h"

void setup() {
  Serial.begin(115200);
  // setup low power definition
  LowPower.setSleepMode(sleep_mode_enum_t::deepSleep);  // or lightSleep
}

void loop() {
  Serial.print("run once");
  LowPower.sleep();
  Serial.print("never called");
}