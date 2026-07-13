#pragma once

#include "LowPowerConfig.h"

#if defined(ESP32)
#  include "LowPowerESP32.h"
#elif defined(ESP8266)
#  include "LowPowerESP8266.h"
#elif defined(ARDUINO_ARCH_RP2040)
#  include "LowPowerRP2040.h"
#elif defined(ARDUINO_ARCH_SAMD)
#  include "LowPowerSAMD.h"
#elif defined(ARDUINO_attiny)
#  include "LowPowerATTiny.h"
#else
#  error The library is not compatible with your board
#endif

// For Arduino, automatically add namespace (see config)
#if LOW_POWER_USING_NS
using namespace low_power;
#endif