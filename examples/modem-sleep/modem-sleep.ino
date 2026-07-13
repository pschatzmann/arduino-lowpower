/**
 * @brief modemSleep minimizes the modem power usage: In addtion
 * we can define a sleep time to stop any other processing as well.
 * This is not supported by all processors!
 * 
 * @author Phil Schatzmann
 */

#include "LowPower.h"

const char* ssid = "SSID";
const char* password = "PWD";

void login() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected");
}

void setup() {
  Serial.begin(115200);
  // login to Wifi
  login();
  LowPower.setSleepTime(5, time_unit_t::sec);
  // set modemSleep is supported
  if (!LowPower.setSleepMode(sleep_mode_enum_t::modemSleep)){
    Serial.println("modemSleep not supported");
  }
}

void loop() {
  Serial.print("processing...");
  // sleeping for 5 sec
  LowPower.process();  
}