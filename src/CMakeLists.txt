add_executable(pico_lora_mqtt_gateway main.cpp config_manager.cpp)

target_include_directories(pico_lora_mqtt_gateway
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
  pico_lora_mqtt_gateway pico_stdlib pico_cyw43_arch_lwip_threadsafe_background
  pico_lwip_http pico_lwip_mqtt hardware_flash)

target_link_libraries(pico_lora_mqtt_gateway cjson)

# USB serial output
pico_enable_stdio_usb(pico_lora_mqtt_gateway 0)
pico_enable_stdio_uart(pico_lora_mqtt_gateway 1)

# Generate UF2 firmware
pico_add_uf2_output(pico_lora_mqtt_gateway)

# Print firmware info
add_custom_command(
  TARGET pico_lora_mqtt_gateway
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E cmake_echo_color --cyan
    "Firmware ready:  ${CMAKE_CURRENT_BINARY_DIR}/pico_lora_mqtt_gateway.uf2")
