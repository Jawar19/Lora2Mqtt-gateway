#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <cstdint>
extern "C" {
#include "dhcpserver.h"
}
#include <cstdio>
#include <cyw43.h>
#include <cyw43_ll.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>

enum class WifiMode : uint8_t { OFF, STA, AP };

enum class wifi_status : uint8_t {
  IDLE,
  SCANNING,
  CONNECTING,
  CONNECTED,
  FAILED,
  AP_MODE
};

struct BlinkConfig {
  int      times    = 1;   ///< Number of blinks
  uint32_t delay_ms = 200; ///< Delay between on/off (ms)
};

class WifiManager {
private:
  static inline bool          _initialized  = false;
  static inline WifiMode      _current_mode = WifiMode::OFF;
  static inline dhcp_server_t _dhcp_server;

public:
  WifiManager() = delete;

  static bool init(WifiMode mode) {
    if (!_initialized) {
      if (cyw43_arch_init()) {
        printf("ERROR: Wi-Fi hardware init failed\n");
        return false;
      }
      _initialized = true;
    }

    if (mode == WifiMode::AP) {
      cyw43_arch_enable_ap_mode(nullptr, nullptr, 0);
    } else {
      cyw43_arch_enable_sta_mode();
    }
    _current_mode = mode;
    return true;
  };

  static bool connect_sta(const char *ssid, const char *password,
                          uint32_t timeout_ms = 10000) {
    if (_current_mode != WifiMode::STA) {
      printf("ERROR: Must be in STA mode to connect.\n");
      return false;
    }

    int result = cyw43_arch_wifi_connect_timeout_ms(
        ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms);

    return result == 0;
  };

  static bool start_ap(const char *ssid, const char *password) {
    if (!_initialized) {
      printf("ERROR: WiFi not initialized.\n");
      return false;
    }

    if (_current_mode != WifiMode::AP) {
      printf("ERROR: Cannot start AP while in %d mode.\n", (int)_current_mode);
      return false;
    };
    cyw43_arch_enable_ap_mode(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t ip, mask, gw;
    IP4_ADDR(&ip, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 4, 1);

    struct netif *netif = &cyw43_state.netif[CYW43_ITF_AP];
    netif_set_addr(netif, &ip, &mask, &gw);
    dhcp_server_init(&_dhcp_server, &ip, &mask);
    printf("AP Mode: Hosting %s at %s\n", ssid, ip4addr_ntoa(&ip));

    return true;
  };

  static void

      static void
      disable() {
    printf("Wifi disable %s mode\n",
           (_current_mode == WifiMode::AP) ? "AP" : "STA");
    if (_current_mode == WifiMode::AP) {
      cyw43_arch_disable_ap_mode();
    } else {
      cyw43_arch_disable_sta_mode();
    }
  }

  static void deinit() {
    printf("Deinit Wifi hardware\n");
    cyw43_arch_deinit();
  }

  static bool is_connected() {
    struct netif *netif =
        &cyw43_state.netif[(_current_mode == WifiMode::AP) ? CYW43_ITF_AP
                                                           : CYW43_ITF_STA];
    if (netif == nullptr) {
      return false;
    }
    if (!netif_is_up(netif)) {
      return false;
    }
    if (ip4_addr_isany(netif_ip4_addr(netif))) {
      return false;
    }

    return true;
  }

  static const char *get_ip() {
    struct netif *netif =
        &cyw43_state.netif[(_current_mode == WifiMode::AP) ? CYW43_ITF_AP
                                                           : CYW43_ITF_STA];
    return ip4addr_ntoa(netif_ip4_addr(netif));
  }

  static void poll() { cyw43_arch_poll(); }

  static void set_led(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
  }

  static void blink_led(BlinkConfig cfg = {}) {
    for (int i = 0; i < cfg.times; ++i) {
      set_led(true);
      sleep_ms(cfg.delay_ms);
      set_led(false);
      sleep_ms(cfg.delay_ms);
    }
  }
};
#endif // !WIFI_MANAGER_H
