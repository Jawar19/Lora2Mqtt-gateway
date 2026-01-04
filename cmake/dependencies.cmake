include(FetchContent)
find_package(Patch REQUIRED)

message(STATUS "Fetching Pico SDK from source ...")
FetchContent_Declare(
  pico_sdk
  GIT_REPOSITORY https://github.com/raspberrypi/pico-sdk.git
  GIT_TAG 2.2.0
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/pico-sdk)

FetchContent_Populate(pico_sdk)

message(STATUS "Fetching MQTT-C from source ...")
FetchContent_Declare(
  mqtt_c
  GIT_REPOSITORY https://github.com/LiamBindle/MQTT-C.git
  GIT_TAG v1.1.6
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/mqtt-c)
FetchContent_MakeAvailable(mqtt_c)

message(STATUS "Fetching cJSON (JSON library) from source ...")

FetchContent_Declare(
  cJSON
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG v1.7.19
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/cJSON
  PATCH_COMMAND "${Patch_EXECUTABLE}" -p1 <
                ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cjson.patch
  UPDATE_DISCONNECTED 1
  EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(cJSON)

message(STATUS "All dependencies fetched!")
