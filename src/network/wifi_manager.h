#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <cstdint>

#include <cyw43_ll.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>

extern "C" {
#include "dhcpserver.h"
}

enum class WifiMode : uint8_t { MODE_AP, MODE_STA, NONE };

enum class WifiState : uint8_t {
  STATE_OFF = 0,
  STATE_INITIALIZING,
  STATE_IDLE,
  STATE_SCANNING,
  STATE_CONNECTING,
  STATE_CONNECTED,
  STATE_DISCONNECTED,
  STATE_ERROR_AUTH,
  STATE_ERROR_NOT_FOUND,
  STATE_ERROR_UNKNOWN
};

struct BlinkConfig {
  int      times    = 1;   ///< Number of blinks
  uint32_t delay_ms = 200; ///< Delay between on/off (ms)
};

class WifiManager {

public:
  WifiManager();
  ~WifiManager();

  bool init(WifiMode mode);
  bool start_ap(const char *ssid, const char *password);
  bool connect_sta(const char *ssid, const char *password,
                   uint32_t timeout_ms = 10000);
  void start_scan();

  void disable();
  void deinit();

  [[nodiscard]] WifiState get_state() const;

  static WifiManager *get_callback_instance() { return _instance; }

private:
  static WifiManager *_instance;

  dhcp_server_t _dhcp_server;
  WifiState     _state = WifiState::STATE_OFF;
  WifiMode      _mode  = WifiMode::NONE;
};
#endif // !WIFI_MANAGER_H
