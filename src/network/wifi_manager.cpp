#include "wifi_manager.h"
#include <cassert>
#include <cstdio>
#include <cyw43.h>
#include <vector>

WifiManager             *WifiManager::_instance = nullptr;
std::vector<WifiNetwork> WifiManager::scan_result;

WifiManager::WifiManager() {
  printf("Wifi Ctor called\n");
  _instance = this;
  _state    = WifiState::STATE_OFF;
  _mode     = WifiMode::NONE;
}

WifiManager::~WifiManager() {
  printf("Wifi dtor called\n");
  if (_instance == this) {
    _instance = nullptr;
  }
  if (_state != WifiState::STATE_OFF) {
    deinit();
  }
}

bool WifiManager::init(WifiMode mode) {
  if (_state == WifiState::STATE_OFF) {
    if (cyw43_arch_init()) {
      printf("ERROR: Wi-Fi hardware init failed\n");
      return false;
    }
    _state = WifiState::STATE_IDLE;
  }

  if (mode == WifiMode::MODE_AP) {
    cyw43_arch_enable_ap_mode(nullptr, nullptr, 0);
  } else {
    cyw43_arch_enable_sta_mode();
  }
  _mode = mode;
  return true;
};

bool WifiManager::start_ap(const char *ssid, const char *password) {
  if (_state == WifiState::STATE_OFF) {
    printf("ERROR: WiFi not initialized.\n");
    return false;
  }

  if (_mode != WifiMode::MODE_AP) {
    printf("ERROR: Cannot start AP while in %d mode.\n", (int)_mode);
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

bool WifiManager::connect_sta(const char *ssid, const char *password,
                              uint32_t timeout_ms) {
  assert("NOT IMPEMENTED YET!!");
  return false;
}

bool WifiManager::start_scan() {
  if (_mode != WifiMode::MODE_STA) {
    printf("ERROR: Wifi is not in STA mode, cannot start SSID SCAN\n");
    return false;
  }
  if (cyw43_wifi_scan_active(&cyw43_state)) {
    printf("WARNING: Scanning already in progress!\n");
    return true;
  }
  cyw43_wifi_scan_options_t opts = {0};
  int err = cyw43_wifi_scan(&cyw43_state, &opts, nullptr, scan_result_callback);

  if (err == 0) {
    printf("WiFi Scan started...\n");
    return true;
  }

  return false;
}

bool WifiManager::is_scanning() {
  return cyw43_wifi_scan_active(&cyw43_state);
}

void WifiManager::poll() {
  cyw43_poll();
}

void WifiManager::disable() {
  printf("Wifi disable %s mode\n", (_mode == WifiMode::MODE_AP) ? "AP" : "STA");
  if (_mode == WifiMode::MODE_AP) {
    cyw43_arch_disable_ap_mode();
  } else {
    cyw43_arch_disable_sta_mode();
  }
  _state = WifiState::STATE_IDLE;
}

void WifiManager::deinit() {
  printf("Deinit Wifi hardware\n");
  cyw43_arch_deinit();
  _state = WifiState::STATE_OFF;
}

WifiState WifiManager::get_state() const {
  return this->_state;
}
WifiMode WifiManager::get_mode() const {
  return this->_mode;
}

int WifiManager::scan_result_callback(void                         *env,
                                      const cyw43_ev_scan_result_t *result) {

  if (result->ssid_len < 1) {
    return 0;
  }
  auto existing_network = std::ranges::find_if(
      scan_result.begin(), scan_result.end(), [&](const WifiNetwork &net) {
        return net.ssid == (const char *)result->ssid;
      });

  if (existing_network != scan_result.end()) {
    if (result->rssi > existing_network->rssi) {
      existing_network->rssi    = result->rssi;
      existing_network->channel = result->channel;
    }
  } else {
    scan_result.push_back(
        {(const char *)result->ssid, result->rssi, result->channel});
  }
  return 0;
}

//
//   static bool connect_sta(const char *ssid, const char *password,
//                           uint32_t timeout_ms = 10000) {
//     if (_current_mode != WifiMode::STA) {
//       printf("ERROR: Must be in STA mode to connect.\n");
//       return false;
//     }
//
//     int result = cyw43_arch_wifi_connect_timeout_ms(
//         ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms);
//
//     return result == 0;
//   };
//
//   //
//   static void
//

//

//
//   static bool is_connected() {
//     struct netif *netif =
//         &cyw43_state.netif[(_current_mode == WifiMode::AP) ? CYW43_ITF_AP
//                                                            :
//                                                            CYW43_ITF_STA];
//     if (netif == nullptr) {
//       return false;
//     }
//     if (!netif_is_up(netif)) {
//       return false;
//     }
//     if (ip4_addr_isany(netif_ip4_addr(netif))) {
//       return false;
//     }
//
//     return true;
//   }
//
//   static const char *get_ip() {
//     struct netif *netif =
//         &cyw43_state.netif[(_current_mode == WifiMode::AP) ? CYW43_ITF_AP
//                                                            :
//                                                            CYW43_ITF_STA];
//     return ip4addr_ntoa(netif_ip4_addr(netif));
//   }
//
//   static void poll() { cyw43_arch_poll(); }
//
//   static void set_led(bool on) {
//     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
//   }
//
//   static void blink_led(BlinkConfig cfg = {}) {
//     for (int i = 0; i < cfg.times; ++i) {
//       set_led(true);
//       sleep_ms(cfg.delay_ms);
//       set_led(false);
//       sleep_ms(cfg.delay_ms);
//     }
//   }
// };
//
