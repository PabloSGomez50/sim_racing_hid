#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

/* Must match the AP credentials configured on Pico2W. */
#define UDP_WIFI_SSID "SIMRACE_AP"
#define UDP_WIFI_PASS "simrace123"

#define UDP_REMOTE_IP "192.168.4.1"
#define UDP_REMOTE_PORT 7777

#define UDP_STA_IP "192.168.4.2"
#define UDP_STA_GATEWAY "192.168.4.1"
#define UDP_STA_NETMASK "255.255.255.0"

#endif
