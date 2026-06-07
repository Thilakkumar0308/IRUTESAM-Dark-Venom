// ════════════════════════════════════════════════════════════
// wifi_module.cpp
// Safe defensive Wi-Fi tools ONLY:
//   - Scan nearby networks
//   - Show SSID, BSSID, RSSI, channel, encryption
//   - Security analysis
//   - Local port scanner (common ports only)
// ════════════════════════════════════════════════════════════

#include "wifi_module.h"
#include <WiFi.h>
#include <WiFiClient.h>

WiFiNetwork wifiNets[WIFI_MAX_NETWORKS];
uint8_t     wifiNetCount = 0;
WiFiNetwork selectedNet;

static const uint16_t SCAN_PORTS[WIFI_PORT_COUNT] = {
    21, 22, 23, 53, 80, 443, 8080
};

void wifiModuleInit() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("[WiFi] Module ready");
}

uint8_t wifiScan(WiFiNetwork *nets, uint8_t maxNets) {
    Serial.println("[WiFi] Scanning...");
    int found = WiFi.scanNetworks(false, true);
    if (found <= 0) {
        Serial.println("[WiFi] No networks found");
        return 0;
    }

    uint8_t count = (uint8_t)min((int)maxNets, found);
    for (uint8_t i = 0; i < count; i++) {
        strncpy(nets[i].ssid,  WiFi.SSID(i).c_str(),  WIFI_SSID_MAX  - 1);
        strncpy(nets[i].bssid, WiFi.BSSIDstr(i).c_str(), WIFI_BSSID_MAX - 1);
        nets[i].ssid [WIFI_SSID_MAX  - 1] = '\0';
        nets[i].bssid[WIFI_BSSID_MAX - 1] = '\0';
        nets[i].rssi    = WiFi.RSSI(i);
        nets[i].channel = WiFi.channel(i);

        wifi_auth_mode_t auth = WiFi.encryptionType(i);
        if      (auth == WIFI_AUTH_OPEN)     nets[i].secType = SEC_OPEN;
        else if (auth == WIFI_AUTH_WEP)      nets[i].secType = SEC_WEP;
        else if (auth == WIFI_AUTH_WPA_PSK)  nets[i].secType = SEC_WPA;
        else if (auth == WIFI_AUTH_WPA3_PSK) nets[i].secType = SEC_WPA3;
        else                                 nets[i].secType = SEC_WPA2;
    }

    WiFi.scanDelete();
    Serial.printf("[WiFi] Found %u networks\n", count);
    return count;
}

uint8_t wifiPortScan(const char *ip, PortScanResult *results, uint8_t maxResults) {
    uint8_t count = min((uint8_t)WIFI_PORT_COUNT, maxResults);
    WiFiClient client;
    client.setTimeout(400);

    for (uint8_t i = 0; i < count; i++) {
        results[i].port = SCAN_PORTS[i];
        results[i].open = client.connect(ip, SCAN_PORTS[i]);
        if (results[i].open) client.stop();
        delay(10);
    }
    return count;
}

const char* wifiSecName(int secType) {
    switch (secType) {
        case SEC_OPEN:  return "OPEN";
        case SEC_WEP:   return "WEP";
        case SEC_WPA:   return "WPA";
        case SEC_WPA2:  return "WPA2";
        case SEC_WPA3:  return "WPA3";
        default:        return "UNK";
    }
}

uint8_t wifiSignalBars(int rssi) {
    if (rssi > -60) return 3;
    if (rssi > -75) return 2;
    return 1;
}
