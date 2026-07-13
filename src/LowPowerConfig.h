#pragma once

/// Automatically add using namespace
#ifndef LOW_POWER_USING_NS
#  define LOW_POWER_USING_NS 1
#endif

/// Activate / deactivate log
//#define LP_LOG(x) { Serial.println(x); Serial.flush(); }
#define LP_LOG(x) 