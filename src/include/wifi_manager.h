#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <array>
#include <cstdio>
#include <cyw43.h>
#include <cyw43_ll.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>

#include <cstddef>

enum class wifi_status : uint8_t {
  DISCONNECTED = 0,
  CONNECTING,
  CONNECTED,
  ERROR
};

struct blink_config {
  int      times    = 1;   ///< Number of blinks
  uint32_t delay_ms = 200; ///< Delay between on/off (ms)
};

static inline bool wifi_init() {
  if (cyw43_arch_init()) {
    printf("ERROR: Wi-Fi initialization failed.\n");
    return false;
  }
  cyw43_arch_enable_sta_mode();
  return true;
}

static inline bool wifi_connect(const char *ssid, const char *password,
                                uint32_t timeout_ms) {
  int result = cyw43_arch_wifi_connect_timeout_ms(
      ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms);
  if (result != 0) {
    printf("ERROR, Wi-Fi connection failed with code: %d\n", result);
    return false;
  }
  return true;
}

static inline const char *wifi_get_ip(char *buffer, size_t buf_size) {
  struct netif *netif = netif_default;
  if (netif == nullptr || !netif_is_up(netif)) {
    return nullptr;
  }
  snprintf(buffer, buf_size, "%s", ip4addr_ntoa(netif_ip4_addr(netif)));
  return buffer;
}

static inline void wifi_print_status() {
  std::array<char, 16> ip_buff;
  const char          *ip = wifi_get_ip(ip_buff.data(), sizeof(ip_buff));

  if (ip) {
    printf("Wi-Fi Status: Connected\n");
    printf("\t%20s:%20s\n", "IP Address", ip);
    printf("\t%20s:%20s\n", "Netmask",
           ip4addr_ntoa(netif_ip4_netmask(netif_default)));
    printf("\t%20s:%20s\n", "Gateway",
           ip4addr_ntoa(netif_ip4_gw(netif_default)));
  } else {
    printf("Wi-Fi Status: Disconnected\n");
  }
}

static inline bool wifi_is_connected() {
  struct netif *netif = netif_default;
  return netif != nullptr && netif_is_up(netif) &&
         ip4_addr_isany(netif_ip4_addr(netif));
}

static inline int wifi_get_link_status() {
  return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
}

static inline void wifi_disconnect() { cyw43_arch_disable_sta_mode(); }

static inline void wifi_deinit() { cyw43_arch_deinit(); }

static inline void wifi_poll() { cyw43_arch_poll(); }

static inline void wifi_blink_led(blink_config cfg = {}) {
  for (int i = 0; i < cfg.times; ++i) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
    sleep_ms(cfg.delay_ms);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
    sleep_ms(cfg.delay_ms);
  }
}

static inline void wifi_set_led(bool on) {
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

#endif // !WIFI_MANAGER_H
