#include "config_manager.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <hardware/flash.h>
#include <hardware/regs/addressmap.h>
#include <hardware/sync.h>
#include <pico/critical_section.h>
#include <pico/types.h>

ConfigManager::ConfigManager() {
  critical_section_init(&this->crit_sec_);
}

ConfigManager::~ConfigManager() {
  critical_section_deinit(&this->crit_sec_);
}

bool ConfigManager::exists() {
  return validate_header();
};

bool ConfigManager::erase_all() {
  uint32_t status = save_and_disable_interrupts();

  flash_range_erase(config::CONFIG_OFFSET, config::SECTOR_SIZE);

  restore_interrupts_from_disabled(status);

  const uint8_t *flash_ptr = get_flash_ptr();
  for (size_t i = 0; i < config::SECTOR_SIZE; ++i) {
    if (flash_ptr[i] != 0xFF) {
      printf("ERROR: Erase verification failed at offset %zu\n", i);
      return false;
    }
  }
  return true;
}

bool ConfigManager::wifi_load(wifi_config &cfg) {
  critical_section_enter_blocking(&this->crit_sec_);
  if (!wifi_cache_valid_) {
    // read_from_flash()
  }
  critical_section_exit(&this->crit_sec_);
  return true;
}

const uint8_t *ConfigManager::get_flash_ptr() {
  const uintptr_t flash_addr =
      static_cast<uintptr_t>(XIP_BASE) + config::CONFIG_OFFSET;
  // This is unavoidable for memory-mapped hardware
  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  return reinterpret_cast<const uint8_t *>(flash_addr);
}

bool ConfigManager::validate_header() {
  const uint8_t *flash_ptr = get_flash_ptr();
  const auto *header = reinterpret_cast<const flash_header *>(get_flash_ptr());

  if (header->magic != config::MAGIC) {
    return false;
  }

  if (header->version != config::VERSION) {
    return false;
  }

  uint32_t stored_crc;
  memcpy(&stored_crc, &flash_ptr[config::SECTOR_SIZE - 4], 4);

  uint32_t calculated_crc32 =
      calculate_crc32(flash_ptr, config::SECTOR_SIZE - 4);

  if (stored_crc != calculated_crc32) {
    printf("CRC mismatch! Stored=0x%08X, Calculated=0x%08X", stored_crc,
           calculated_crc32);
    return false;
  }

  return true;
}

uint32_t ConfigManager::calculate_crc32(const uint8_t *data, size_t len) {
  uint32_t crc = 0xFFFFFFFF;
  if (len == 0) {
    return crc;
  }

  for (size_t i = 0; i < len; ++i) {
    uint8_t index = (crc ^ data[i]) & 0xFF;
    crc           = (crc >> 8) ^ config_detail::crc_table[index];
  }

  return ~crc;
}

void ConfigManager::read_config_sector(uint8_t *buffer) {
  const uint8_t *flash_ptr = get_flash_ptr();
  std::memcpy(buffer, flash_ptr, config::SECTOR_SIZE);
}

bool ConfigManager::write_config_sector(const uint8_t *buffer) {
  uint32_t nvec = save_and_disable_interrupts();

  flash_range_erase(config::CONFIG_OFFSET, config::SECTOR_SIZE);
  flash_range_program(config::CONFIG_OFFSET, buffer, config::SECTOR_SIZE);

  restore_interrupts(nvec);

  const uint8_t *flash_ptr = get_flash_ptr();
  return (memcmp(flash_ptr, buffer, config::SECTOR_SIZE) == 0);
}
