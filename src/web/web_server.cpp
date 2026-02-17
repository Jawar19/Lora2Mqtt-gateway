#include "web_server.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include <charconv>
#include <cstdio>
#include <cstring>
#include <lwip/apps/httpd.h>
#include <lwip/def.h>

static WebServer *g_web_bridge     = nullptr;
const char *WebServer::_ssi_tags[] = {"setup_flag", "wifi_list", "ssid_target"};
bool        WebServer::_is_setup_mode = false;

WebServer::WebServer(WifiManager *wifi, ConfigManager *config) {
  printf("Webserver CTOR called!\n");
  this->_wifi_mgr = wifi;
  this->config    = config;
  g_web_bridge    = this;
}

WebServer::~WebServer() {
  printf("Webserver DTOR called!\n");
  g_web_bridge = nullptr;
}

void WebServer::init() {
  httpd_init();
  http_set_ssi_handler(_ssi_handler, _ssi_tags, LWIP_ARRAYSIZE(_ssi_tags));
  http_set_cgi_handlers(cgi_handlers, 1);
}

void WebServer::set_setup_mode(bool is_setup) {
  _is_setup_mode = is_setup;
}

u16_t WebServer::_ssi_handler(int iIndex, char *pcInsert, int iInsertLen,
                              u16_t current_tag_part, u16_t *next_tag_part) {

  if (!g_web_bridge || !g_web_bridge->_wifi_mgr) {
    return 0;
  }

  u16_t printed = 0;

  switch (iIndex) {
  case 0: // Setup_flag
  {
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
  case 2: // ssid_target
  {
    printed = snprintf(pcInsert, iInsertLen, "%s\n",
                       g_web_bridge->_wifi_mgr->pending_request.ssid.c_str());
    break;
  }
  default:
    break;
  }

  return printed;
}

const char *WebServer::cgi_wifi_connect_handler(int iIndex, int iNumParams,
                                                char *pcParam[],
                                                char *pcValue[]) {
  std::string ssid;
  std::string password;

  for (int i = 0; i < iNumParams; ++i) {
    if (strcmp(pcParam[i], "ssid") == 0) {
      g_web_bridge->_wifi_mgr->pending_request.ssid = _url_decode(pcValue[i]);
    }
    if (strcmp(pcParam[i], "pwd") == 0) {
      g_web_bridge->_wifi_mgr->pending_request.password =
          _url_decode(pcValue[i]);
    }
  }

  g_web_bridge->_wifi_mgr->pending_request.cmd = WifiCommand::CONNECT_NEW;

  return "/connecting.shtml";
}

const tCGI WebServer::cgi_handlers[] = {
    // remember to update CGI handler length in init
    {
        "/wifi_connect.cgi",     // The name in the HTML form
        cgi_wifi_connect_handler // The function we just wrote
    },
};

std::string WebServer::_url_decode(std::string_view text) {
  std::string decoded;
  decoded.reserve(text.size()); // Optimization: prevent multiple reallocations

  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '%') {
      if (i + 2 < text.size()) {
        int value;
        // std::from_chars is the modern, fast C++ way to parse hex
        auto result = std::from_chars(text.data() + i + 1, text.data() + i + 3,
                                      value, 16);

        if (result.ec == std::errc{}) {
          decoded += static_cast<char>(value);
          i += 2;
          continue;
        }
      }
    } else if (text[i] == '+') {
      decoded += ' ';
    } else {
      decoded += text[i];
    }
  }
  return decoded;
}
