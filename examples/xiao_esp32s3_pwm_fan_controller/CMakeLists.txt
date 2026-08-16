cmake_minimum_required(VERSION 3.16)

# Interactive fan-PWM profile for the XIAO ESP32-S3 and external encoder
# console. The active profile uses D3 PWM and D4 tach feedback; the legacy C6
# power-switch/Matter driver remains in the tree but is deliberately excluded.
set(IDF_TARGET "esp32s3" CACHE STRING "ESP-IDF target")
set(PROJECT_VER "0.3.0")
set(PROJECT_VER_NUMBER 3)

# ESP-IDF otherwise compiles every project component even when the active main
# component does not require it. Keep the legacy full fan driver source present
# but outside this focused XIAO S3 interaction build.
set(EXCLUDE_COMPONENTS fan_control)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_12v_pwm_fan)

idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
