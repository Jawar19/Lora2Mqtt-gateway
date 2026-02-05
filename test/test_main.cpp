#include "CppUTest/CommandLineTestRunner.h"
#include <cstdio>
#include <hardware/gpio.h>
#include <pico/stdio.h>
#include <pico/time.h>

#define PICO_DEFAULT_LED_PIN 25;

int main(int argc, char *argv[]) {
  stdio_init_all();

  // Wait for USB serial to be ready
  sleep_ms(2000);

  printf("\n\n");
  printf("============================================\n");
  printf("  LoRa2Mqtt Gateway - Hardware-In-Loop Tests\n");
  printf("  Running on %s\n", PICO_BOARD);
  printf("============================================\n\n");

  // Run tests with verbose output
  const char *test_args[] = {"tests", "-v", "-c"};
  int result = CommandLineTestRunner::RunAllTests(3, (char **)test_args);

  printf("\n============================================\n");
  if (result == 0) {
    printf("✓ ALL TESTS PASSED\n");
  } else {
    printf("✗ TESTS FAILED (return code: %d)\n", result);
  }
  printf("============================================\n\n");

  // Blink LED to indicate result
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  // while (true) {
  //   // Pass = slow blink, Fail = fast blink
  //   int delay_ms = (result == 0) ? 1000 : 200;
  //   gpio_put(LED_PIN, 1);
  //   sleep_ms(delay_ms);
  //   gpio_put(LED_PIN, 0);
  //   sleep_ms(delay_ms);
  // }

  return result;
}
