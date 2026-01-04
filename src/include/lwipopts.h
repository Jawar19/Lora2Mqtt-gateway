#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Common settings for Pico W
#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define LWIP_NETIF_API 0

// Memory settings
#define MEM_LIBC_MALLOC 0
#define MEMP_NUM_TCP_SEG 32
#define MEMP_NUM_ARP_QUEUE 10
#define PBUF_POOL_SIZE 24
#define LWIP_ARP 1
#define LWIP_ETHERNET 1
#define LWIP_ICMP 1
#define LWIP_RAW 1
#define TCP_MSS 1460
#define TCP_WND (8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)
#define TCP_SND_QUEUELEN ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_NETCONN 0
#define MEM_STATS 0
#define SYS_STATS 0
#define MEMP_STATS 0
#define LINK_STATS 0

// DHCP
#define LWIP_DHCP 1
#define LWIP_IPV4 1
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DNS 1
#define LWIP_TCP_KEEPALIVE 1

// MQTT specific settings
#define LWIP_MQTT 1
#define MQTT_REQ_MAX_IN_FLIGHT 5
#define MQTT_CYCLIC_TIMER_INTERVAL 5

// HTTP server (for your web UI)
#define LWIP_HTTPD 1
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI 1
#define LWIP_HTTPD_SSI_MULTIPART 1
#define HTTPD_SERVER_PORT 80

// Debug (disable for production)
#define LWIP_DEBUG 0
#define TCP_DEBUG LWIP_DBG_OFF
#define MQTT_DEBUG LWIP_DBG_OFF

// Platform-specific
#define LWIP_RAND() ((u32_t)rand())

#endif /* _LWIPOPTS_H */
