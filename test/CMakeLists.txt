set(TEST_NAME "test")
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

include(FetchContent)
FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG master
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/cpputest)
set(TESTS
    OFF
    CACHE BOOL "Switch off CppUTest Test build")
FetchContent_MakeAvailable(CppUTest)

include(${CMAKE_SOURCE_DIR}/cmake/files.cmake)

add_executable(${TEST_NAME} test_main.cpp test_config_manager.cpp ${SRC_FILES})

target_include_directories(${TEST_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

target_link_libraries(${TEST_NAME} PRIVATE pico_stdlib CppUTest::CppUTest
                                           CppUTest::CppUTestExt)

target_compile_definitions(
  ${TEST_NAME} PRIVATE PICO_CXX_DISABLE_ALLOCATION_OVERRIDES
                       CPPUTEST_HAVE_EXCEPTIONS=1)

# enable usb output, disable uart output
pico_enable_stdio_usb(${TEST_NAME} 1)
pico_enable_stdio_uart(${TEST_NAME} 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(${TEST_NAME})
