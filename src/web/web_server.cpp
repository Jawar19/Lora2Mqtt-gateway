#include "web_server.h"
#include "wifi_manager.h"
#include <cstdio>
#include <lwip/apps/httpd.h>
#include <lwip/def.h>

static WebServer *g_web_bridge              = nullptr;
const char       *WebServer::_ssi_tags[]    = {"setup_flag", "wifi_list"};
bool              WebServer::_is_setup_mode = false;

WebServer::WebServer(WifiManager *mgr) {
  printf("Webserver CTOR called!\n");
  this->_wifi_mgr = mgr;
  g_web_bridge    = this;
}

WebServer::~WebServer() {
  printf("Webserver DTOR called!\n");
  g_web_bridge = nullptr;
}

void WebServer::init() {
  httpd_init();
  http_set_ssi_handler(_ssi_handler, _ssi_tags, LWIP_ARRAYSIZE(_ssi_tags));
}

void WebServer::set_setup_mode(bool is_setup) {
  _is_setup_mode = is_setup;
}

u16_t WebServer::_ssi_handler(int iIndex, char *pcInsert, int iInsertLen,
                              u16_t current_tag_part, u16_t *next_tag_part) {

  printf("SSI request received in backend\n");
  if (!g_web_bridge || !g_web_bridge->_wifi_mgr) {
    return 0;
  }

  u16_t printed = 0;

  switch (iIndex) {
  case 0: // Setup_flag
  {
    printf("is_setup_mode %u\n", _is_setup_mode ? 1 : 0);
    printed = snprintf(pcInsert, iInsertLen, "%d", _is_setup_mode ? 1 : 0);
    break;
  }
  case 1: // Wifi list
  {
    auto net_idx = (size_t)current_tag_part;

    if (WifiManager::scan_result.empty()) {
      break;
    }

    auto &network = WifiManager::scan_result[net_idx];
    printed       = snprintf(
        pcInsert, iInsertLen, "<option value=\"%s\">%s (%d dBm)</option>\n",
        network.ssid.c_str(), network.ssid.c_str(), network.rssi);

    if (net_idx + 1 < WifiManager::scan_result.size()) {
      *next_tag_part = current_tag_part + 1;
    }
    break;
  }
  default:
    break;
  }

  return printed;
}
