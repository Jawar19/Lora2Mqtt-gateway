cmake_minimum_required(VERSION 3.16)
set(PICO_SDK_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/pico-sdk"
    CACHE PATH "Path to Pico SDK")
unset(PICO_EXAMPLES_PATH CACHE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-toolchain.cmake)

set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2_w)

# Include the pre-init file (must happen BEFORE project())
include(${CMAKE_CURRENT_SOURCE_DIR}/extern/pico-sdk/pico_sdk_init.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

project(
  pico_lora_mqtt_gateway
  LANGUAGES C CXX ASM
  VERSION 0.1.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror=format")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

add_subdirectory(src)

message(STATUS "")
message(STATUS "=== LoRa868-to-MQTT Gateway Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Pico SDK:  ${PICO_SDK_PATH}")
message(STATUS "Target Board: ${PICO_BOARD}")
message(STATUS "============================================")
message(STATUS "")
