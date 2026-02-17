#include <algorithm>
#include <cctype>
#include <cstring>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>
#include <hardware/structs/io_bank0.h>
#include <hardware/uart.h>
#include <hardware/watchdog.h>
#include <memory>
#include <pico/stdio.h>
#include <pico/stdio_uart.h>

#include <cstdio>

#include "config_manager.h"
#include "web_server.h"
#include <algorithm>
#include <pico/time.h>
#include <string>
#include <uart_cmd_handler.h>
#include <wifi_manager.h>

#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BAUD_RATE 115200
#define WIFI_CONNECT_LIMIT 3

using state_t = enum {
  BOOTING,
  INITIAL_SETUP,
  WIFI_START_SCAN,
  WIFI_IS_SCANNING,
  WIFI_START_AP,
  WIFI_CONNECT_STA,
  WEBSERVER_INITIALIZE,
  MONITORING,
  SHUTDOWN,
  REBOOT,
  RESET,
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
    state = REBOOT;
    // Trigger restart
  } else if (cmd == "reset") {
    printf("Resetting stored config and rebooting...\n");
    state = RESET;
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

  uint8_t wifi_connection_attemps = 1;

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
  std::unique_ptr<WebServer>   web;

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
      printf("is_ApMode: %s\n", cfg.is_ApMode() ? "true" : "false");
      if (cfg.is_ApMode()) {
        state = WIFI_START_SCAN;
      } else {
        state = WIFI_CONNECT_STA;
      }
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
        std::ranges::sort(WifiManager::scan_result,
                          [](const WifiNetwork &a, const WifiNetwork &b) {
                            return a.rssi > b.rssi;
                          });
        for (const WifiNetwork &net : WifiManager::scan_result) {
          printf("SSID:%32s | Channel: %2d | RSSI: %d\n", net.ssid.c_str(),
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
    case WIFI_CONNECT_STA: {
      wifi_config w_cfg = cfg.wifi();
      if (wifi->connect_sta(cfg.wifi().ssid.data(),
                            cfg.wifi().password.data())) {
        state = MONITORING;
      } else {
        printf("Could not connect to %s (%d/%d), reattempting ~15s \n",
               cfg.wifi().ssid.data(), wifi_connection_attemps,
               WIFI_CONNECT_LIMIT);
        wifi_connection_attemps += 1;
        sleep_ms(15000);
      };
      if (wifi_connection_attemps > WIFI_CONNECT_LIMIT) {
        printf("Could not connect to %s! Resetting config...\n",
               cfg.wifi().ssid.data());
        state = RESET;
      }
      break;
    }
    case WEBSERVER_INITIALIZE:
      if (!web) {
        web = std::make_unique<WebServer>(wifi.get(), &cfg);
        WebServer::init();
        WebServer::set_setup_mode(true);
      }
      state = MONITORING;
      break;

    case MONITORING:
      if (wifi->pending_request.cmd == WifiCommand::CONNECT_NEW) {
        wifi_config wifi_cfg;

        wifi_cfg.ssid.fill(0);
        wifi_cfg.password.fill(0);

        strncpy(wifi_cfg.ssid.data(), wifi->pending_request.ssid.c_str(),
                wifi_cfg.ssid.size());
        strncpy(wifi_cfg.password.data(),
                wifi->pending_request.password.c_str(),
                wifi_cfg.password.size());
        cfg.wifi_save(wifi_cfg);
        cfg.clear_flags(config::ConfigFlags::WifiApMode);
        state = REBOOT;
      }
      break;
    case SHUTDOWN:
      printf("Shutdown called, cleaning up\n");
      irq_set_enabled(UART0_IRQ, false);
      wifi->disable();
      wifi->deinit();
      break;
    case REBOOT:
      printf("Rebooting gateway in ~2 seconds...\n");
      sleep_ms(2000);
      watchdog_enable(1, true);
      watchdog_reboot(0, 0, 0);
      break;
    case RESET:
      cfg.erase_all();
      state = REBOOT;
      break;
    default:
      printf("unknown state %d", state);
    }

    sleep_ms(10);
  } while (state != SHUTDOWN);
  return 0;
};
