#ifndef web_server_H_
#define web_server_H_

#include <lwip/apps/httpd.h>
#include <lwip/arch.h>

#include "config_manager.h"
#include "wifi_manager.h"

class WebServer {

public:
  WebServer(WifiManager *wifi, ConfigManager *config);

  WebServer &operator=(const WebServer &) = default;

  ~WebServer();

  static void init();

  static void set_setup_mode(bool is_setup);

private:
  WifiManager   *_wifi_mgr;
  ConfigManager *config;
  static bool    _is_setup_mode;

  static std::string _url_decode(std::string_view text);

  static const char *_ssi_tags[3];
  static u16_t       _ssi_handler(int iIndex, char *pcInsert, int iInsertLen,
                                  u16_t current_tag_part, u16_t *next_tag_part);

  static const tCGI  cgi_handlers[];
  static const char *cgi_wifi_connect_handler(int iIndex, int iNumParams,
                                              char *pcParam[], char *pcValue[]);
};

#endif // web_server_H_
