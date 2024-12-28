# Low Power Library

If you want to process sensor readings powered by batteries it is critical that the power usage is minimized to extendd the battery life. This can be achieved by different means (e.g. reducing the cpu speed, adjusting the voltage) or by sleeping the processor. 


This library allows the use of the low power features for the following architechtures:

- ESP32
- ESP8266
- RP2040
- SAMD

We support the sleep modes lightSleep and deepSleep: The difference between them is the power saving and wakeup time.

The common API supports the definition of

- the sleep mode
- the active time
- the sleep period
- wakup pins

## Example

Here is an example that sets the processor into deep-sleep after 2 seconds of activity and let's it wake up by setting gpio pin 4 to active.

```C++

void setup() {
  Serial.begin(115200);

  // setup low power definition
  LowPower.setActiveTime(4, time_unit_t::sec);
  LowPower.setSleepMode(sleep_mode_enum_t::deepSleep);
  LowPower.addWakeupPin(1, pin_change_t::on_high);
}

void loop() {
  Serial.print("loop running for 4 sec");
  LowPower.process();
}
```

Further examples can be found [here](examples).


## Documentaion

There is a common API: however each architecture might support some specific additional methods to fine tune the processing:

- [RP2040](https://pschatzmann.github.io/arduino-lowpower/docs/html/classlow__power_1_1ArduinoLowPowerRP2040.html)
- [ESP32](https://pschatzmann.github.io/arduino-lowpower/docs/html/classlow__power_1_1ArduinoLowPowerESP32.html)
- [ESP8266](https://pschatzmann.github.io/arduino-lowpower/docs/html/classlow__power_1_1ArduinoLowPowerESP8266.html)
- [SAMD](https://pschatzmann.github.io/arduino-lowpower/docs/html/classlow__power_1_1ArduinoLowPowerSAMD.html)

## Project Status

The project is compiling w/o errors, [but not all functionality has been tested](https://github.com/pschatzmann/arduino-lowpower/wiki/Testing-Status).
Any help and feedback is welcome!
