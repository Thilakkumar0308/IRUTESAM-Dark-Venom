#ifndef TFT_UI_H
#define TFT_UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

extern TFT_eSPI tft;
extern TFT_eSprite ui;

// Init
void tftInit();
void drawBootScreen();

// Core text / chrome
void drawCenteredText(const char *text, int y, uint16_t color, uint8_t size);
void drawStatusBar(const char *name, uint16_t nameColor);
void drawFooter(const char *h1, const char *h2, const char *h3);

// Menu
void drawMenu(const char *items[], uint8_t count,
              uint8_t sel, uint8_t offset, const char *title);
void drawMenuRowPartial(const char *label, int rowY, bool selected);

// Dialog
void drawDialog(const char *title, const char *b1, const char *b2,
                bool yesSelected);
bool checkDialogTouch(bool &yesSelected);

// WiFi
void drawWifiList(const char **ssids, const int *rssi,
                  const int *secTypes, uint8_t count,
                  uint8_t sel, uint8_t offset);
void drawWifiDetail(const char *ssid, const char *bssid,
                    int rssi, uint8_t channel, int secType);
void drawDeauthScreen(const char *ssid, uint8_t channel,
                      uint32_t pktCount, bool pulse);
void drawDeauthProtect(uint8_t deauthCount);
void drawSecurityAnalysis(const char **ssids, const int *secTypes,
                          uint8_t count, uint8_t sel, uint8_t offset);

// NFC
void drawNFCScan(uint8_t step);
void drawNFCResult(const char *uid, uint8_t uidLen);
void drawNFCEmulate(const char *uid, bool pulse);
void drawNFCSavedList(const char **items, uint8_t count,
                      uint8_t sel, uint8_t offset);

// IR
void drawIRCapture(uint16_t *rawData, uint16_t rawLen);
void drawIRSignalInfo(const char *proto, uint32_t addr, uint32_t cmd);
void drawIRSend(bool pulse);
void drawIRSavedList(const char **items, uint8_t count,
                     uint8_t sel, uint8_t offset);

// Files
void drawFileList(const char *title, const char **items,
                  uint8_t count, uint8_t sel, uint8_t offset);

// Settings
void drawBrightness(int level);
void drawAbout();

// Touch cal
void drawTouchCalCross(uint8_t step, int x, int y);

#endif
