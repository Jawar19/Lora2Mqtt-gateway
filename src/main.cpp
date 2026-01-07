#include <cstdint>
#include <pico/stdio.h>

#include <cstdio>

#include "config_manager.h"

auto main(int argc, char *argv[]) -> int {

  stdio_init_all();
  printf("Launching Lora2MQTT Gateway\n");

  // test memory pointer
  auto       &cfg       = ConfigManager::instance();
  const auto *flash_ptr = ConfigManager::get_flash_ptr();
  printf("Flash Config pointer: %p\n", flash_ptr);
  printf("First 4 bytes of config space:\n %02X %02X %02X %02X\n", flash_ptr[0],
         flash_ptr[1], flash_ptr[2], flash_ptr[3]);
  return 0;
}
