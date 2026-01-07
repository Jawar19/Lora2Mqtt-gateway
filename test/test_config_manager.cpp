/***
 * Testing the config manager functions
 */
#include "config_manager.h"
#include <CppUTest/TestHarness.h>
#include <array>
#include <cstdint>
#include <cstdio>

TEST_GROUP(configGroup){};

TEST(configGroup, crc32_empty) {
  const uint8_t empty[] = {};
  uint32_t      result  = ConfigManager::calculate_crc32(empty, 0);
  printf("EMPTY result: 0x%8X", result);
  CHECK_EQUAL(0xFFFFFFFF, result); // Empty data should return initial XOR
}

TEST(configGroup, crc32_single_byte) {
  const uint8_t data[] = {0x00};
  uint32_t      result = ConfigManager::calculate_crc32(data, 1);
  CHECK_EQUAL(0xD202EF8D, result); // Known CRC32 for single 0x00 byte
}

TEST(configGroup, crc32_known_string) {
  const char *str    = "123456789"; // Common test string
  uint32_t    result = ConfigManager::calculate_crc32((const uint8_t *)str, 9);
  CHECK_EQUAL(0xCBF43926, result); // Well-known CRC32 of "123456789"
}
