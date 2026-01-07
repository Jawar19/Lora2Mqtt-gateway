#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "pico/stdlib.h"
#include <cstdio>
#include <pico/stdio.h>

int main(int argc, char *argv[]) {
  stdio_init_all();

  printf("Running Tests\n");
  int result = CommandLineTestRunner::RunAllTests(argc, argv);

  printf("Runner returned %d\n", result);

  return result;
}
