include(${CMAKE_SOURCE_DIR}/cmake/files.cmake)

add_executable(pico_lora_mqtt_gateway main.cpp ${SRC_FILES})

target_include_directories(
  pico_lora_mqtt_gateway
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}/network
          ${CMAKE_CURRENT_SOURCE_DIR}/uart
          ${CMAKE_CURRENT_SOURCE_DIR}/network/dhcp
          ${CMAKE_CURRENT_SOURCE_DIR}/web
          ${CMAKE_CURRENT_SOURCE_DIR}/config)

pico_add_library(pico_httpd_content NOFLAG)
pico_set_lwip_httpd_content(
  pico_httpd_content
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/web/html/style.css
  ${CMAKE_CURRENT_SOURCE_DIR}/web/html/index.shtml
  ${CMAKE_CURRENT_SOURCE_DIR}/web/html/favicon.svg
  ${CMAKE_CURRENT_SOURCE_DIR}/web/html/wifi_initialize.shtml)

target_link_libraries(
  pico_lora_mqtt_gateway
  pico_stdlib
  pico_cyw43_arch_lwip_threadsafe_background
  pico_lwip_http
  pico_lwip_mqtt
  pico_httpd_content
  hardware_flash
  cjson)

# USB serial output (0 for disabled, 1 for enabled)
pico_enable_stdio_usb(pico_lora_mqtt_gateway 0)
pico_enable_stdio_uart(pico_lora_mqtt_gateway 1)

# Generate UF2 firmware
pico_add_uf2_output(pico_lora_mqtt_gateway)

# -----------------------------------------------

# Print firmware info
add_custom_command(
  TARGET pico_lora_mqtt_gateway
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E cmake_echo_color --cyan
    "Firmware ready:  ${CMAKE_CURRENT_BINARY_DIR}/pico_lora_mqtt_gateway.uf2")
