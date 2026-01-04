include(FetchContent)
find_package(Patch REQUIRED)

message(STATUS "Fetching cJSON (JSON library) from source ...")

set(ENABLE_CJSON_TEST
    OFF
    CACHE BOOL "Disable cJSON tests")
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "Build static libraries only")
set(BUILD_SHARED_AND_STATIC_LIBS
    OFF
    CACHE BOOL "Disable shared libs")

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
