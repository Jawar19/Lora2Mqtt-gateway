#include "web_server.h"
#include "wifi_manager.h"
#include <array>
#include <lwip/apps/httpd.h>
#include <lwip/def.h>

const std::array<const char *, 2> WebServer::_ssi_tags = {"wifi_list",
                                                          "wifi_status"};
WebServer::WebServer(WifiManager *mgr) {
  this->_wifi_mgr = mgr;
  httpd_init();

  // http_set_ssi_handler(WebServer::_ssi_handler, WebServer::_ssi_tags.data(),
  //                      WebServer::_ssi_tags.size());
}

u16_t WebServer::_ssi_handler(int iIndex, char *pcInsert, int iInsertLen,
                              u16_t current_tag_part, u16_t *next_tag_part) {
  size_t printed = 0;

  switch (iIndex) {
  case 0: // "wifi_list"

    break;
  case 1: // "wifi_status"
    printed = snprintf(pcInsert, iInsertLen, "System Ready");
    break;
  default:
    break;
  }

  return 0;
}
