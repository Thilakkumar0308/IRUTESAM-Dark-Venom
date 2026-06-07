#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>
#include "config.h"

#define WIFI_MAX_NETWORKS   40
#define WIFI_SSID_MAX       33
#define WIFI_BSSID_MAX      18
#define WIFI_PORT_COUNT      7

struct WiFiNetwork {
    char    ssid[WIFI_SSID_MAX];
    char    bssid[WIFI_BSSID_MAX];
    int     rssi;
    uint8_t channel;
    int     secType;
};

struct PortScanResult {
    uint16_t port;
    bool     open;
};

void    wifiModuleInit();
uint8_t wifiScan(WiFiNetwork *nets, uint8_t maxNets);
uint8_t wifiPortScan(const char *ip, PortScanResult *results, uint8_t maxResults);
const char* wifiSecName(int secType);
uint8_t wifiSignalBars(int rssi);

extern WiFiNetwork wifiNets[WIFI_MAX_NETWORKS];
extern uint8_t     wifiNetCount;
extern WiFiNetwork selectedNet;

#endif
