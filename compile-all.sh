#!/bin/bash

#
# Test compile for all supported architectures
#

function compile_example {
    ARCH=$1
    FILE="./examples/wakeup-pin"
    # take action on each file. $f store current file name
    arduino-cli compile -b "$ARCH" "$FILE"
    EC=$?
    echo -e "$ARCH $FILE -> rc=$EC" >> "compile-all-log.txt"
}

rm compile-all-log.txt
compile_example "esp32:esp32:esp32" 
compile_example "esp32:esp32:esp32c3" 
compile_example "esp32:esp32:esp32s3" 
compile_example "esp32:esp32:esp32s2" 
compile_example "esp32:esp32:esp32c6" 
compile_example "esp32:esp32:esp32h2" 
compile_example "esp8266:esp8266:generic" 
compile_example "rp2040:rp2040:generic" 
compile_example "arduino:samd:arduino_zero_native"
#compile_example "STMicroelectronics:stm32:GenF4" 
