#include <cctype>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/uart.h>
#include <memory>
#include <pico/stdio.h>
#include <pico/stdio_uart.h>

#include <cstdio>

#include "config_manager.h"
#include <string>
#include <uart_cmd_handler.h>
#include <wifi_manager.h>

#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BAUD_RATE 115200

using state_t = enum {
  BOOTING,
  INITIAL_SETUP,
  WIFI_START_SCAN,
  WIFI_IS_SCANNING,
  WIFI_START_AP,
  WEBSERVER_INITIALIZE,
  SHUTDOWN,
};

state_t state = BOOTING;

void process_command(const std::string &cmd) {
  printf("\nProcessing command: %s\n", cmd.c_str());

  // Add your command handlers here
  if (cmd == "status") {
    printf("Gateway status: Running\n");
  } else if (cmd == "scan") {
    printf("Starting LoRa scanner mode...\n");
    // Enable scanner mode
  } else if (cmd == "whitelist") {
    printf("Showing whitelist...\n");
    // Display whitelist
  } else if (cmd == "restart") {
    printf("Restarting gateway...\n");
    // Trigger restart
  } else if (cmd == "exit") {
    state = SHUTDOWN;

  } else if (cmd == "help") {
    printf("Available commands:\n");
    printf("  status    - Show gateway status\n");
    printf("  scan      - Enter scanner mode\n");
    printf("  whitelist - Show whitelisted devices\n");
    printf("  restart   - Restart gateway\n");
    printf("  help      - Show this help\n");
  } else {
    printf("Unknown command: %s (type 'help' for commands)\n", cmd.c_str());
  }

  printf("> "); // Prompt for next command
}

auto main(int argc, char *argv[]) -> int {

  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  stdio_uart_init_full(UART_ID, BAUD_RATE, UART_TX_PIN, UART_RX_PIN);

  sleep_ms(1000);
  printf("\n\n*==============================*\n");
  printf("* Launching Lora2MQTT Gateway! *\n");
  printf("*==============================*\n\n");

  auto                         uart_cmd = UARTCommandHandler(UART_ID);
  auto                        &cfg      = ConfigManager::instance();
  std::unique_ptr<WifiManager> wifi     = std::make_unique<WifiManager>();

  if (!cfg.exists()) {
    cfg.create_defaults(); // Writes to flash
  }

  if (!cfg.init()) {
    panic("Config init failed");
  }

  do {
    if (UARTCommandHandler::has_command()) {
      std::string cmd = uart_cmd.get_command();
      process_command(cmd);
    }

    switch (state) {
    case BOOTING:
      printf("BOOTING\n");
      if (cfg.is_ApMode()) {
        state = WIFI_START_SCAN;
      }
      break;
    case WEBSERVER_INITIALIZE:
      // TODO Initialise the webserver and serve the Connect to wifi page
      break;
    case WIFI_START_SCAN:
      wifi->init(WifiMode::MODE_STA);
      if (!wifi->start_scan()) {
        state = SHUTDOWN;
        break;
      }
      state = WIFI_IS_SCANNING;
      break;
    case WIFI_IS_SCANNING:
      if (!WifiManager::is_scanning()) {
        for (WifiNetwork &net : WifiManager::scan_result) {
          printf("SSID:%32s | Channel: %d | RSSI: %d\n", net.ssid.c_str(),
                 net.channel, net.rssi);
        }
        state = WIFI_START_AP;
      }
      break;
    case WIFI_START_AP:
      printf("Creating AP for initial configuration!\n");
      printf("SSID: %s\n", cfg.wifi().ssid.data());
      wifi->init(WifiMode::MODE_AP);
      if (wifi->start_ap(cfg.wifi().ssid.data(), cfg.wifi().password.data())) {
        state = WEBSERVER_INITIALIZE;
      } else {
        state = SHUTDOWN;
      }

      break;
    case SHUTDOWN:
      printf("Shutdown called, cleaning up\n");
      irq_set_enabled(UART0_IRQ, false);
      wifi->disable();
      wifi->deinit();
      break;

    default:
      printf("unknown state %d", state);
    }

    sleep_ms(10);
  } while (state != SHUTDOWN);
  return 0;
};
