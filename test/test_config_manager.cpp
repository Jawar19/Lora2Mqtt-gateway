/***
 * Testing the config manager functions
 */
#include "CppUTest/TestHarness_c.h"
#include "config_manager.h"
#include <CppUTest/TestHarness.h>
#include <array>
#include <cstdint>
#include <cstdio>

// Test accessor class - wraps private methods
class ConfigManagerTestAccess {
public:
  // Expose private methods for testing
  static uint32_t calculate_crc32(const uint8_t *data, size_t len) {
    return ConfigManager::calculate_crc32(data, len);
  }

  static bool validate_header() { return ConfigManager::validate_header(); }

  static const uint8_t *get_flash_ptr() { return ConfigManager::get_flash_ptr(); }

  static void read_config_sector(uint8_t *buffer) { ConfigManager::read_config_sector(buffer); }

  static bool write_config_sector(const uint8_t *buffer) {
    return ConfigManager::write_config_sector(buffer);
  }
};

// clang-format off
TEST_GROUP(configManager){};
TEST_GROUP(configManagerFlash){
  void setup() override{
    // Optional:  Erase flash before each test
    ConfigManager &cm = ConfigManager::instance();
    cm.erase_all();
  }

  void teardown() override {
  // Cleanup if needed
  }
};
// clang-format on

TEST(configManager, crc32_empty) {
  const uint8_t empty[] = {};
  uint32_t result = ConfigManagerTestAccess::calculate_crc32(empty, 0);
  CHECK(0 == result); // Empty data should return initial XOR
}

TEST(configManager, crc32_single_byte) {
  const uint8_t data[] = {0x00};
  uint32_t result = ConfigManagerTestAccess::calculate_crc32(data, 1);
  CHECK_EQUAL(0xD202EF8D, result); // Known CRC32 for single 0x00 byte
}

TEST(configManager, crc32_known_string) {
  const char *str = "123456789"; // Common test string
  uint32_t result = ConfigManagerTestAccess::calculate_crc32((const uint8_t *)str, 9);
  CHECK_EQUAL(0xCBF43926, result); // Well-known CRC32 of "123456789"
}

TEST(configManagerFlash, clears_all_bytes_to_ff) {
  ConfigManager &cm = ConfigManager::instance();

  bool result = cm.erase_all();
  CHECK_TRUE(result);

  const uint8_t *flash_ptr = ConfigManagerTestAccess::get_flash_ptr();
  for (size_t i = 0; i < config::SECTOR_SIZE; i++) {
    if (flash_ptr[i] != 0xFF) {
      printf("Byte at offset %zu is 0x%02X, expected 0xFF\n", i, flash_ptr[i]);
      FAIL("Flash not properly erased");
    }
  }
}
