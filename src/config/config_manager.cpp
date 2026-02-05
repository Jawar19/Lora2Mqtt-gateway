#include "config_manager.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <hardware/flash.h>
#include <hardware/regs/addressmap.h>
#include <hardware/sync.h>
#include <pico/critical_section.h>
#include <pico/platform/panic.h>
#include <pico/types.h>
#include <span>

ConfigManager::ConfigManager() {
  critical_section_init(&this->crit_sec_);
  this->cache_valid_ = false;
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

  restore_interrupts(status);

  const std::byte *flash_ptr = get_flash_ptr();
  for (size_t i = 0; i < config::SECTOR_SIZE; ++i) {
    if (flash_ptr[i] != std::byte{0xFF}) {
      printf("ERROR: Erase verification failed at offset %zu\n", i);
      return false;
    }
  }
  cache_valid_ = false;
  return true;
}

bool ConfigManager::create_defaults() {
  critical_section_enter_blocking(&crit_sec_);

  flash_layout layout = create_default_layout();

  auto layout_view = std::as_bytes(std::span{&layout, 1});

  uint32_t verify_crc =
      calculate_crc32(layout_view.first<config::CRC_OFFSET>());
  if (verify_crc != layout.crc) {
    critical_section_exit(&crit_sec_);
    printf("Could not create default config, CRC mismatch\n");
    return false;
  }

  bool success = write_config_sector(layout_view);

  critical_section_exit(&crit_sec_);
  return success;
};
bool ConfigManager::init() {
  if (!validate_header()) {
    printf("Error: could not validate config\n");
    cache_valid_ = false;
    return false;
  }

  if (!reload_cache()) {
    printf("ERROR: Failed to load config from flash\n");
    cache_valid_ = false;
    return false;
  }

  cache_valid_ = true;
  return true;
};

const wifi_config &ConfigManager::wifi() const {
  if (!cache_valid_) {
    panic("Attempted to access invalid cache!");
  }
  return layout_cache_.wifi;
};
bool ConfigManager::wifi_save(const wifi_config &cfg) {
  if (!cache_valid_) {
    printf("ERROR: Cannot save - cache not initialized\n");
    return false;
  }
  layout_cache_.wifi = cfg;
  return flush_cache();
};

const mqtt_config &ConfigManager::mqtt() const {
  if (!cache_valid_) {
    panic("Attempted to access invalid cache!");
  }
  return layout_cache_.mqtt;
};
bool ConfigManager::mqtt_save(const mqtt_config &cfg) {
  if (!cache_valid_) {
    printf("ERROR: Cannot save - cache not initialized\n");
    return false;
  }
  layout_cache_.mqtt = cfg;
  return flush_cache();
};

const lora_config &ConfigManager::lora() const {
  if (!cache_valid_) {
    panic("Attempted to access invalid cache!");
  }
  return layout_cache_.lora;
};
bool ConfigManager::lora_save(const lora_config &cfg) {
  if (!cache_valid_) {
    printf("error: cannot save - cache not initialized\n");
    return false;
  }
  layout_cache_.lora = cfg;
  return flush_cache();
};

const uint8_t *ConfigManager::whitelist() const {
  if (!cache_valid_) {
    panic("Attempted to access invalid cache!");
  }
  return layout_cache_.whitelist_block;
};
bool ConfigManager::whitelist_save() {
  if (!cache_valid_) {
    printf("error: cannot save - cache not initialized\n");
    return false;
  }
  // layout_cache_.lora = cfg;
  return flush_cache();
};

const uint8_t *ConfigManager::devcfg() const {
  if (!cache_valid_) {
    panic("Attempted to access invalid cache!");
  }
  return layout_cache_.device_configs;
};
bool ConfigManager::devcfg_save() {
  if (!cache_valid_) {
    printf("error: cannot save - cache not initialized\n");
    return false;
  }
  // layout_cache_.lora = cfg;
  return flush_cache();
};

config::ConfigFlags ConfigManager::get_flags() const {
  if (!cache_valid_) {
    printf("WARNING: Reading flags from invalid cache\n");
    return config::ConfigFlags::None;
  }
  return layout_cache_.header.flags;
};

bool ConfigManager::set_flags(config::ConfigFlags new_flags) {
  if (!cache_valid_) {
    printf("ERROR: Cannot modify flags - cache invalid\n");
    return false;
  }

  layout_cache_.header.flags = new_flags;
  return flush_cache();
};

bool ConfigManager::add_flags(config::ConfigFlags flags) {
  return set_flags(get_flags() | flags);
};

bool ConfigManager::clear_flags(config::ConfigFlags flags) {
  return set_flags(get_flags() & ~flags);
};
bool ConfigManager::has_flags(config::ConfigFlags flags) const {
  return (get_flags() & flags) == flags;
};

bool ConfigManager::is_ApMode() const {
  if (!cache_valid_) {
  }
  return has_flags(config::ConfigFlags::WifiApMode);
};

const std::byte *ConfigManager::get_flash_ptr() {
  const auto *base = reinterpret_cast<const std::byte *>(XIP_BASE);
  return base + config::CONFIG_OFFSET; // NOLINT(performance-no-int-to-ptr)
}

bool ConfigManager::validate_header() {
  std::span<const std::byte, config::SECTOR_SIZE> flash_view(
      get_flash_ptr(), config::SECTOR_SIZE);

  flash_header header;
  std::memcpy(&header, flash_view.data(), sizeof(flash_header));

  if (header.magic != config::MAGIC || header.version != config::VERSION) {
    return false;
  }

  uint32_t stored_crc;
  memcpy(&stored_crc, &flash_view[config::CRC_OFFSET], sizeof(uint32_t));

  uint32_t calculated_crc32 =
      calculate_crc32(flash_view.first<config::CRC_OFFSET>());

  if (stored_crc != calculated_crc32) {
    printf("CRC mismatch! Stored=0x%08X, Calculated=0x%08X", stored_crc,
           calculated_crc32);
    return false;
  }

  return true;
}

bool ConfigManager::reload_cache() {

  std::span<const std::byte, config::SECTOR_SIZE> flash_view(
      get_flash_ptr(), config::SECTOR_SIZE);

  uint32_t flash_crc;
  std::memcpy(&flash_crc, &flash_view[config::CRC_OFFSET], sizeof(uint32_t));

  uint32_t calc_crc = calculate_crc32(flash_view.first<config::CRC_OFFSET>());

  if (flash_crc != calc_crc) {

    printf("ERROR: CRC mismatch in flash config\n");
    printf("CRC mismatch! Stored=0x%08X, Calculated=0x%08X", flash_crc,
           calc_crc);
    cache_valid_ = false;
    return false;
  }

  std::memcpy(&this->layout_cache_, flash_view.data(), config::SECTOR_SIZE);

  cache_valid_ = true;
  return true;
};

bool ConfigManager::flush_cache() {
  if (!cache_valid_) {
    printf("ERROR: Cannot flush invalid cache\n");
    return false;
  }
  critical_section_enter_blocking(&crit_sec_);
  layout_cache_.header.write_count++;

  auto buffer_view = std::as_bytes(std::span{&layout_cache_, 1});

  layout_cache_.crc = calculate_crc32(buffer_view.first<config::CRC_OFFSET>());

  bool success = write_config_sector(buffer_view);
  critical_section_exit(&crit_sec_);
  return success;
};

uint32_t ConfigManager::calculate_crc32(std::span<const std::byte> data) {
  uint32_t crc = 0xFFFFFFFF;

  for (const std::byte byte : data) {
    uint8_t index = (crc ^ static_cast<uint8_t>(byte)) & 0xFF;
    crc           = (crc >> 8) ^ config_detail::crc_table[index];
  }

  return ~crc;
}

void ConfigManager::read_config_sector(uint8_t *buffer) {
  std::memcpy(buffer, get_flash_ptr(), config::SECTOR_SIZE);
}

bool ConfigManager::write_config_sector(std::span<const std::byte> buffer) {
  uint32_t nvec = save_and_disable_interrupts();

  flash_range_erase(config::CONFIG_OFFSET, config::SECTOR_SIZE);

  flash_range_program(config::CONFIG_OFFSET,
                      reinterpret_cast<const uint8_t *>(buffer.data()),
                      config::SECTOR_SIZE);

  restore_interrupts(nvec);

  return (memcmp(get_flash_ptr(), buffer.data(), config::SECTOR_SIZE) == 0);
}

namespace {
template <size_t N>
void safe_string_copy(std::array<char, N> &dest, const char *src) {
  if (src == nullptr) {
    dest[0] = '\0';
    return;
  }
  size_t len = std::min(std::strlen(src), N - 1);
  std::copy_n(src, len, dest.data());
  dest[len] = '\0';
}
} // namespace

flash_layout ConfigManager::create_default_layout() {
  flash_layout layout = {};

  layout.header.magic       = config::MAGIC;
  layout.header.version     = config::VERSION;
  layout.header.flags       = {config::ConfigFlags::WifiApMode};
  layout.header.write_count = 0;

  safe_string_copy(layout.wifi.ssid, config::DEFAULT_SSID.data());
  safe_string_copy(layout.wifi.password, "LoRa123456");

  safe_string_copy(layout.mqtt.broker, "192.168.1.100");
  layout.mqtt.port = 1883;
  safe_string_copy(layout.mqtt.base_topic, "lora/gateway");

  layout.lora.frequency        = 433050000;
  layout.lora.spreading_factor = 7;
  layout.lora.bandwidth        = 125;
  layout.lora.coding_rate      = 5;
  layout.lora.tx_power         = 14;

  auto layout_view = std::as_bytes(std::span{&layout, 1});
  layout.crc       = calculate_crc32(layout_view.first<config::CRC_OFFSET>());

  return layout;
}
