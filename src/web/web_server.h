#ifndef web_server_H_
#define web_server_H_

#include <array>
#include <cstdint>
#include <lwip/arch.h>

#include "wifi_manager.h"

class WebServer {

  static const std::array<const char *, 2> _ssi_tags;

public:
  WebServer(WifiManager *mgr);

  WebServer &operator=(const WebServer &) = default;

  ~WebServer();

private:
  WifiManager      *_wifi_mgr;
  static WebServer *_instance;
  static u16_t      _ssi_handler(int iIndex, char *pcInsert, int iInsertLen,
                                 u16_t current_tag_part, u16_t *next_tag_part);
  uint16_t          cgi_scan_wifi_handler();
};

#endif // web_server_H_
