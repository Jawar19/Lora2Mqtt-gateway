cmake_minimum_required(VERSION 3.16)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED PICO_SDK_PATH)
  if(DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH
        $ENV{PICO_SDK_PATH}
        CACHE PATH "Path to Pico SDK")
  else()
    message(
      FATAL_ERROR
        "PICO_SDK_PATH not set. Please set environment variable or pass -DPICO_SDK_PATH=/path/to/pico-sdk"
    )
  endif()
endif()

set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2_w)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  pico_lora_mqtt_gateway
  LANGUAGES C CXX ASM
  VERSION 0.1.1)

pico_sdk_init()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

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
message(STATUS "PICO_NO_FLASH: ${PICO_NO_FLASH}")
message(STATUS "============================================")
message(STATUS "")
