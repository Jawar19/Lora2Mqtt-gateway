#ifndef web_server_H_
#define web_server_H_

#include <lwip/arch.h>

#include "wifi_manager.h"

class WebServer {

public:
  WebServer(WifiManager *mgr);

  WebServer &operator=(const WebServer &) = default;

  ~WebServer();

  static void init();

  static void set_setup_mode(bool is_setup);

private:
  WifiManager *_wifi_mgr;
  static bool  _is_setup_mode;

  static const char *_ssi_tags[2];
  static u16_t       _ssi_handler(int iIndex, char *pcInsert, int iInsertLen,
                                  u16_t current_tag_part, u16_t *next_tag_part);
};

#endif // web_server_H_
