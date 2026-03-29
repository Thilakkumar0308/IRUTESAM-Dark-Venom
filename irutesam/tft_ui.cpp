#include "tft_ui.h"
#include "touch_input.h"
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();

extern bool wifiReady;
extern bool sdAvailable;
extern bool nfcAvailable;
extern bool touchAvailable;
extern int  directRowIndex;

static void clearContentArea() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
}

static void drawSimpleButton(int x, int y, int w, int h,
                             const char *label,
                             uint16_t border,
                             uint16_t fill,
                             uint16_t textCol,
                             uint8_t textSize = 1) {
    tft.fillRoundRect(x, y, w, h, 3, fill);
    tft.drawRoundRect(x, y, w, h, 3, border);
    tft.setTextSize(textSize);
    tft.setTextColor(textCol, fill);
    int tw = tft.textWidth(label);
    int th = (textSize == 1) ? 8 : 16;
    tft.setCursor(x + (w - tw) / 2, y + (h - th) / 2);
    tft.print(label);
}

static void hexPoint(int cx, int cy, int r, int i, int &px, int &py) {
    float a = i * 60.0f * DEG_TO_RAD - DEG_TO_RAD * 90.0f;
    px = cx + (int)(r * cos(a));
    py = cy + (int)(r * sin(a));
}

void tftInit() {
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(DV_BG);
    tft.setTextColor(DV_CYAN, DV_BG);
    tft.setTextSize(1);
}

void drawCenteredText(const char *text, int y, uint16_t color, uint8_t size) {
    tft.setTextSize(size);
    tft.setTextColor(color, DV_BG);
    int w = tft.textWidth(text);
    tft.setCursor((SCREEN_W - w) / 2, y);
    tft.print(text);
}

// ===== EXACT HTML-STYLE BOOT =====
void drawBootScreen() {
    tft.fillScreen(DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(4, 312);
    tft.print("tap screen to skip");
    delay(200);

    const int cx = 120;
    const int cy = 92;

    for (int i = 0; i < 6; i++) {
        int x1, y1, x2, y2;
        hexPoint(cx, cy, 52, i, x1, y1);
        hexPoint(cx, cy, 52, i + 1, x2, y2);
        tft.drawLine(x1, y1, x2, y2, DV_CYAN);
        delay(80);
    }
    delay(100);

    for (int i = 0; i < 6; i++) {
        int x1, y1, x2, y2;
        hexPoint(cx, cy, 42, i, x1, y1);
        hexPoint(cx, cy, 42, i + 1, x2, y2);
        tft.drawLine(x1, y1, x2, y2, DV_CYAN_MID);
    }
    delay(80);

    tft.drawCircle(cx, cy, 58, DV_CYAN_DIM);
    tft.drawCircle(cx, cy, 64, DV_CYAN_MID);

    for (int i = 0; i < 12; i++) {
        float a = i * 30.0f * DEG_TO_RAD;
        uint16_t col = (i % 2 == 1) ? DV_CYAN_DIM : DV_CYAN_MID;
        tft.drawLine(cx + (int)(58 * cos(a)), cy + (int)(58 * sin(a)),
                     cx + (int)(62 * cos(a)), cy + (int)(62 * sin(a)), col);
    }
    delay(120);

    const char *brand = "IRUTESAM";
    tft.setTextSize(2);
    tft.setTextColor(DV_CYAN, DV_BG);
    for (int i = 0; i < 8; i++) {
        char c[2] = { brand[i], '\0' };
        tft.setCursor(72 + i * 12, 84);
        tft.print(c);
        delay(55);
    }
    delay(80);

    const char *ver = "v1.0";
    tft.setTextSize(2);
    tft.setTextColor(DV_PURPLE, DV_BG);
    for (int i = 0; i < 4; i++) {
        char c[2] = { ver[i], '\0' };
        tft.setCursor(96 + i * 12, 108);
        tft.print(c);
        delay(60);
    }
    delay(150);

    drawCenteredText("IRUTESAM", 148, DV_CYAN_MID, 1); delay(80);
    drawCenteredText("IRUTESAM", 148, DV_CYAN_DIM, 1); delay(80);
    drawCenteredText("IRUTESAM", 148, DV_CYAN, 1); delay(60);
    drawCenteredText("DARK VENOM", 162, DV_PURPLE, 1); delay(60);

    for (int x = 35; x <= 200; x += 8) {
        tft.drawFastHLine(35, 174, x - 35, DV_CYAN_MID);
        delay(18);
    }
    delay(100);

    struct BootLog {
        uint16_t c1;
        const char *t1;
        uint16_t c2;
        const char *t2;
    };

    BootLog logs[] = {
        { DV_GREEN, "[OK] ", DV_CYAN_DIM, "WiFi module ready" },
        { DV_GREEN, "[OK] ", DV_CYAN_DIM, "SD card mounted" },
        { DV_GREEN, "[OK] ", DV_CYAN_DIM, "PN532 NFC online" },
        { DV_GREEN, "[OK] ", DV_CYAN_DIM, "IR receiver ready" },
        { DV_GREEN, "[OK] ", DV_CYAN_DIM, "Touch screen ready" },
        { DV_CYAN,  "",      DV_CYAN,     "> Booting IRUTESAM v1.0" }
    };

    int logY = 180;
    for (auto &log : logs) {
        tft.setTextSize(1);
        tft.setCursor(6, logY);
        if (log.t1[0]) {
            tft.setTextColor(log.c1, DV_BG);
            tft.print(log.t1);
        }
        tft.setTextColor(log.c2, DV_BG);
        tft.print(log.t2);
        logY += 13;
        delay(360);
    }

    for (int i = 0; i < 4; i++) {
        tft.setTextColor(DV_CYAN, DV_BG);
        tft.setCursor(6, logY);
        tft.print("_");
        delay(200);
        tft.setTextColor(DV_BG, DV_BG);
        tft.setCursor(6, logY);
        tft.print("_");
        delay(200);
    }

    delay(600);
}

void drawStatusBar(const char *name, uint16_t nameColor) {
    tft.fillRect(0, 0, SCREEN_W, STATUS_H, DV_BG2);
    tft.drawFastHLine(0, STATUS_H, SCREEN_W, DV_CYAN_MID);

    tft.setTextSize(1);
    tft.setTextColor(nameColor, DV_BG2);
    tft.setCursor(6, 4);
    tft.print(name);

    tft.fillRect(196, 6, 4, 4, wifiReady ? DV_CYAN : DV_CYAN_DIM);
    tft.fillRect(206, 6, 4, 4, sdAvailable ? DV_CYAN : DV_CYAN_DIM);
    tft.fillRect(216, 6, 4, 4, nfcAvailable ? DV_PURPLE : DV_CYAN_DIM);
    tft.fillRect(226, 6, 4, 4, touchAvailable ? DV_CYAN : DV_AMBER);
}

void drawFooter(const char *h1, const char *h2, const char *h3) {
    tft.fillRect(0, FOOTER_Y, SCREEN_W, FOOTER_H, DV_BG2);
    tft.drawFastHLine(0, FOOTER_Y, SCREEN_W, DV_CYAN_MID);

    tft.setTextSize(1);
    tft.setTextColor(DV_CYAN, DV_BG2);
    tft.setCursor(4, 308);
    tft.print(h1);

    tft.setTextColor(DV_CYAN_DIM, DV_BG2);
    tft.print(" ");
    tft.print(h2);

    if (h3 && h3[0]) {
        tft.setCursor(170, 308);
        tft.print(h3);
    }

    if (touchAvailable) tft.fillCircle(230, 308, 4, DV_CYAN_DIM);
}

void drawMenuRowPartial(const char *label, int rowY, bool selected) {
    if (selected) {
        tft.fillRect(0, rowY, 234, MENU_ROW_H - 2, DV_HILIGHT);
        tft.setTextColor(DV_CYAN, DV_HILIGHT);
        tft.setTextSize(2);
        tft.setCursor(6, rowY + 8);
        tft.print(">");
        tft.setCursor(20, rowY + 8);
        tft.print(label);
        tft.fillCircle(228, rowY + 17, 3, DV_PINK);
    } else {
        tft.fillRect(0, rowY, 234, MENU_ROW_H - 2, DV_BG);
        tft.setTextColor(DV_CYAN_DIM, DV_BG);
        tft.setTextSize(2);
        tft.setCursor(20, rowY + 8);
        tft.print(label);
    }
}

void drawMenu(const char *items[], uint8_t count,
              uint8_t sel, uint8_t offset, const char *title) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// ");
    tft.print(title);

    for (uint8_t v = 0; v < MENU_MAX_ROWS; v++) {
        uint8_t idx = offset + v;
        if (idx >= count) break;
        int rowY = MENU_START_Y + v * MENU_ROW_H;
        drawMenuRowPartial(items[idx], rowY, idx == sel);
    }

    if (count > MENU_MAX_ROWS) {
        tft.fillRect(236, MENU_START_Y, 2, MENU_ROW_H * MENU_MAX_ROWS, DV_BG2);
        int thumbH = max(4, (MENU_ROW_H * MENU_MAX_ROWS * MENU_MAX_ROWS) / count);
        int thumbY = MENU_START_Y +
                     (offset * (MENU_ROW_H * MENU_MAX_ROWS - thumbH)) / (count - MENU_MAX_ROWS);
        tft.fillRect(236, thumbY, 2, thumbH, DV_CYAN);
    }
}

void drawDialog(const char *title, const char *b1, const char *b2, bool yesSelected) {
    tft.fillRoundRect(20, 95, 200, 95, 4, DV_PANEL);
    tft.drawRoundRect(20, 95, 200, 95, 4, DV_CYAN_MID);
    drawCenteredText(title, 112, DV_PINK, 1);
    tft.drawFastHLine(30, 122, 180, DV_CYAN_MID);
    drawCenteredText(b1, 135, DV_WHITE, 1);
    if (b2 && b2[0]) drawCenteredText(b2, 148, DV_WHITE, 1);

    drawSimpleButton(38, 160, 72, 22, "YES",
        yesSelected ? DV_CYAN : DV_CYAN_MID,
        yesSelected ? DV_HILIGHT : DV_BG2,
        yesSelected ? DV_CYAN : DV_CYAN_DIM, 1);

    drawSimpleButton(130, 160, 72, 22, "NO",
        !yesSelected ? DV_CYAN : DV_CYAN_MID,
        !yesSelected ? DV_HILIGHT : DV_BG2,
        !yesSelected ? DV_CYAN : DV_CYAN_DIM, 1);
}

bool checkDialogTouch(bool &yesSelected) {
    if (!touchAvailable) return false;
    int tx = getLastTouchX();
    int ty = getLastTouchY();

    if (tx >= 38 && tx <= 110 && ty >= 160 && ty <= 182) {
        yesSelected = true;
        return true;
    }
    if (tx >= 130 && tx <= 202 && ty >= 160 && ty <= 182) {
        yesSelected = false;
        return true;
    }
    return false;
}

// ===== WiFi =====
void drawWifiList(const char **ssids, const int *rssi,
                  const int *secTypes, uint8_t count,
                  uint8_t sel, uint8_t offset) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// WiFi Networks");

    for (uint8_t v = 0; v < WIFI_LIST_MAX_VIS; v++) {
        uint8_t idx = offset + v;
        if (idx >= count) break;

        int rowY = WIFI_LIST_START_Y + v * WIFI_LIST_ROW_H;
        bool isSel = (idx == sel);

        if (isSel) tft.fillRect(0, rowY, 210, WIFI_LIST_ROW_H - 2, DV_HILIGHT);

        tft.setTextSize(1);
        tft.setTextColor(isSel ? DV_CYAN : DV_CYAN_DIM, isSel ? DV_HILIGHT : DV_BG);
        tft.setCursor(4, rowY + 4);
        tft.print(ssids[idx]);

        tft.setTextColor(DV_CYAN_DIM, isSel ? DV_HILIGHT : DV_BG);
        tft.setCursor(140, rowY + 4);
        tft.print(rssi[idx]);

        uint16_t bt, bb, bf;
        const char *bl;
        if (secTypes[idx] == SEC_OPEN) {
            bt = DV_RED; bb = DV_RED; bf = DV_RED_FILL; bl = "OPEN";
        } else if (secTypes[idx] == SEC_WEP || secTypes[idx] == SEC_WPA) {
            bt = DV_AMBER; bb = DV_AMBER; bf = DV_AMB_FILL; bl = (secTypes[idx] == SEC_WEP) ? "WEP" : "WPA";
        } else {
            bt = DV_GREEN; bb = DV_GREEN; bf = DV_GRN_FILL; bl = (secTypes[idx] == SEC_WPA3) ? "WPA3" : "WPA2";
        }

        tft.fillRoundRect(155, rowY + 2, 52, 14, 2, bf);
        tft.drawRoundRect(155, rowY + 2, 52, 14, 2, bb);
        tft.setTextColor(bt, bf);
        int bw = tft.textWidth(bl);
        tft.setCursor(155 + (52 - bw) / 2, rowY + 5);
        tft.print(bl);

        int bars = (rssi[idx] > -60) ? 3 : (rssi[idx] > -75) ? 2 : 1;
        for (int b = 0; b < 3; b++) {
            int bh = 4 + b * 3;
            int bx = 216 + b * 5;
            int by = rowY + 14 - bh;
            tft.fillRect(bx, by, 4, bh, b < bars ? DV_CYAN : DV_CYAN_MID);
        }
    }
}

void drawWifiDetail(const char *ssid, const char *bssid, int rssi, uint8_t channel, int secType) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Network Detail");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    auto row = [&](int y, const char *k, const char *v) {
        tft.setTextColor(DV_CYAN_DIM, DV_BG);
        tft.setCursor(6, y);
        tft.print(k);
        tft.setTextColor(DV_CYAN, DV_BG);
        tft.setCursor(70, y);
        tft.print(v);
    };

    char buf[32];
    row(40, "SSID:", ssid);
    row(56, "BSSID:", bssid);
    snprintf(buf, sizeof(buf), "%d dBm", rssi); row(72, "RSSI:", buf);
    snprintf(buf, sizeof(buf), "%u", channel); row(88, "CHANNEL:", buf);

    const char *sec = "UNKNOWN";
    if (secType == SEC_OPEN) sec = "OPEN";
    else if (secType == SEC_WEP) sec = "WEP";
    else if (secType == SEC_WPA) sec = "WPA";
    else if (secType == SEC_WPA2) sec = "WPA2";
    else if (secType == SEC_WPA3) sec = "WPA3";
    row(104, "SECURITY:", sec);

    tft.drawFastHLine(0, 124, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_PINK, DV_BG); tft.setCursor(6, 136); tft.print("SELECT");
    tft.setTextColor(DV_CYAN_DIM, DV_BG); tft.print(" > Launch Deauth Attack");
    tft.setTextColor(DV_GREEN, DV_BG); tft.setCursor(6, 152); tft.print("BACK");
    tft.setTextColor(DV_CYAN_DIM, DV_BG); tft.print(" > Return to list");
}

void drawDeauthScreen(const char *ssid, uint8_t channel, uint32_t pktCount, bool pulse) {
    clearContentArea();
    drawCenteredText("DEAUTH ACTIVE", 40, pulse ? DV_RED : DV_CYAN_DIM, 2);

    int cx = 120, cy = 134;
    if (pulse) tft.fillCircle(cx, cy, 28, DV_RED);
    else {
        tft.fillCircle(cx, cy, 28, DV_BG);
        tft.drawCircle(cx, cy, 28, DV_RED);
    }

    for (int i = 0; i < 6; i++) {
        float a = i * 60.0f * DEG_TO_RAD;
        tft.drawLine(cx + (int)(35 * cos(a)), cy + (int)(35 * sin(a)),
                     cx + (int)(50 * cos(a)), cy + (int)(50 * sin(a)), DV_RED);
    }
    tft.drawCircle(cx, cy, 52, DV_CYAN_MID);

    tft.setTextColor(DV_CYAN_DIM, DV_BG); tft.setCursor(6, 194); tft.print("TARGET:");
    tft.setTextColor(DV_CYAN, DV_BG);     tft.setCursor(60, 194); tft.print(ssid);
    tft.setTextColor(DV_AMBER, DV_BG);    tft.setCursor(6, 210);  tft.print("PKT:");
    tft.setTextColor(DV_RED, DV_BG);      tft.setCursor(40, 210); tft.print(pktCount);

    char ch[12];
    snprintf(ch, sizeof(ch), "CH %u", channel);
    drawCenteredText(ch, 224, DV_PURPLE, 1);
    drawCenteredText("TAP LOWER HALF TO STOP", 272, DV_CYAN_DIM, 1);
}

void drawDeauthProtect(uint8_t deauthCount) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Deauth Protect");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 44);
    tft.print("Monitoring for deauth...");

    tft.setTextColor(DV_AMBER, DV_BG);
    tft.setCursor(6, 70);
    tft.print("DEAUTH FRAMES DETECTED:");

    char buf[8];
    snprintf(buf, sizeof(buf), "%u", deauthCount);
    drawCenteredText(buf, 96, deauthCount > 0 ? DV_RED : DV_GREEN, 3);
    drawCenteredText("BACK to stop monitoring", 262, DV_CYAN_DIM, 1);
}

void drawSecurityAnalysis(const char **ssids, const int *secTypes,
                          uint8_t count, uint8_t sel, uint8_t offset) {
    (void)ssids; (void)sel; (void)offset;
    clearContentArea();

    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Security Analysis");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    int open = 0, weak = 0, strong = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (secTypes[i] == SEC_OPEN) open++;
        else if (secTypes[i] == SEC_WEP || secTypes[i] == SEC_WPA) weak++;
        else strong++;
    }

    tft.setTextColor(DV_CYAN_DIM, DV_BG); tft.setCursor(6, 40); tft.print("Total networks: ");
    tft.setTextColor(DV_CYAN, DV_BG);     tft.print(count);
    tft.setTextColor(DV_RED, DV_BG);      tft.setCursor(6, 58); tft.print("OPEN:   "); tft.print(open);
    tft.setTextColor(DV_AMBER, DV_BG);    tft.setCursor(6, 74); tft.print("WEAK:   "); tft.print(weak);
    tft.setTextColor(DV_GREEN, DV_BG);    tft.setCursor(6, 90); tft.print("SECURE: "); tft.print(strong);

    tft.drawFastHLine(0, 106, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_CYAN_DIM, DV_BG); tft.setCursor(6, 116); tft.print("Risk score:");
    int risk = min(100, open * 25 + weak * 10);
    tft.setTextColor(risk > 50 ? DV_RED : (risk > 20 ? DV_AMBER : DV_GREEN), DV_BG);
    tft.setCursor(90, 116); tft.print(risk); tft.print("%");
    tft.fillRect(6, 132, 228, 10, DV_BG2);
    tft.drawRoundRect(6, 132, 228, 10, 2, DV_CYAN_MID);
    tft.fillRect(6, 132, (228 * risk) / 100, 10,
                 risk > 50 ? DV_RED : (risk > 20 ? DV_AMBER : DV_GREEN));
}

// ===== NFC =====
void drawNFCScan(uint8_t step) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// NFC Scan");

    int cx = 120, cy = 130;
    uint16_t c0, c1, c2;
    switch (step % 3) {
        case 0: c0 = DV_CYAN;     c1 = DV_PURPLE; c2 = DV_CYAN_MID; break;
        case 1: c0 = DV_PURPLE;   c1 = DV_CYAN;   c2 = DV_CYAN_MID; break;
        default:c0 = DV_CYAN_MID; c1 = DV_CYAN;   c2 = DV_PURPLE;   break;
    }

    tft.drawCircle(cx, cy, 18, c0);
    tft.drawCircle(cx, cy, 32, c1);
    tft.drawCircle(cx, cy, 46, c2);
    tft.fillCircle(cx, cy, 4, DV_CYAN);

    for (int i = 0; i < 6; i++) {
        float a = i * 60.0f * DEG_TO_RAD;
        tft.drawLine(cx + (int)(49 * cos(a)), cy + (int)(49 * sin(a)),
                     cx + (int)(54 * cos(a)), cy + (int)(54 * sin(a)), DV_CYAN_DIM);
    }

    tft.fillRoundRect(8, 195, 224, 28, 3, DV_PANEL);
    tft.drawRoundRect(8, 195, 224, 28, 3, DV_CYAN_MID);
    drawCenteredText("SCANNING...", 204, DV_CYAN_DIM, 1);
}

void drawNFCResult(const char *uid, uint8_t uidLen) {
    tft.fillRoundRect(8, 195, 224, 28, 3, DV_PANEL);
    tft.drawRoundRect(8, 195, 224, 28, 3, DV_CYAN_MID);
    tft.setTextSize(2);
    tft.setTextColor(DV_CYAN, DV_PANEL);
    int w = tft.textWidth(uid);
    tft.setCursor((SCREEN_W - w) / 2, 200);
    tft.print(uid);

    char lenBuf[20];
    snprintf(lenBuf, sizeof(lenBuf), "UID LEN: %u bytes", uidLen);
    drawCenteredText(lenBuf, 232, DV_PURPLE, 1);
}

void drawNFCEmulate(const char *uid, bool pulse) {
    clearContentArea();
    tft.fillRoundRect(8, 60, 224, 28, 3, DV_PANEL);
    tft.drawRoundRect(8, 60, 224, 28, 3, DV_CYAN_MID);

    tft.setTextSize(2);
    tft.setTextColor(DV_CYAN, DV_PANEL);
    int w = tft.textWidth(uid);
    tft.setCursor((SCREEN_W - w) / 2, 66);
    tft.print(uid);

    drawCenteredText(pulse ? "BROADCASTING..." : "STANDBY...", 115,
                     pulse ? DV_CYAN : DV_CYAN_DIM, 1);

    int cx = 120, cy = 178;
    tft.drawCircle(cx, cy, 20, DV_PURPLE);
    tft.drawCircle(cx, cy, 35, DV_CYAN);
    tft.drawCircle(cx, cy, 50, DV_CYAN_MID);
    tft.fillCircle(cx, cy, 6, pulse ? DV_PURPLE : DV_CYAN_DIM);
}

void drawNFCSavedList(const char **items, uint8_t count,
                      uint8_t sel, uint8_t offset) {
    drawMenu(items, count, sel, offset, "Saved NFC Cards");
}

// ===== IR =====
void drawIRCapture(uint16_t *rawData, uint16_t rawLen) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PINK, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// IR Capture");

    const int panelX = 8, panelY = 38, panelW = 224, panelH = 58;
    tft.fillRect(panelX, panelY, panelW, panelH, DV_PANEL);
    tft.drawRect(panelX, panelY, panelW, panelH, DV_CYAN_MID);

    if (rawLen == 0) {
        drawCenteredText("WAITING FOR SIGNAL...", 64, DV_CYAN_DIM, 1);
        return;
    }

    int drawX = 10, drawW = 220, midY = 68;
    uint32_t maxVal = 1;
    for (uint16_t i = 0; i < rawLen && i < IR_MAX_RAW; i++)
        if (rawData[i] > maxVal) maxVal = rawData[i];

    int pxPerSample = max(1, drawW / (int)rawLen);
    int xPos = drawX;
    for (uint16_t i = 0; i < rawLen && xPos < drawX + drawW; i++) {
        int w = max(1, (int)((rawData[i] * pxPerSample * 4) / maxVal));
        if (xPos + w > drawX + drawW) w = drawX + drawW - xPos;
        uint16_t col = (i % 2 == 0) ? DV_CYAN : DV_CYAN_MID;
        int barY = (i % 2 == 0) ? 44 : midY + 2;
        int barH = 24;
        tft.fillRect(xPos, barY, w, barH, col);
        xPos += w;
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "RAW SAMPLES: %u", rawLen);
    drawCenteredText(buf, 104, DV_CYAN_DIM, 1);
}

void drawIRSignalInfo(const char *proto, uint32_t addr, uint32_t cmd) {
    tft.fillRoundRect(8, 116, 224, 72, 3, DV_PANEL);
    tft.drawRoundRect(8, 116, 224, 72, 3, DV_CYAN_MID);

    char buf[20];
    tft.setTextSize(1);

    tft.setTextColor(DV_PURPLE, DV_PANEL); tft.setCursor(16, 126); tft.print("PROTO");
    tft.drawFastVLine(70, 120, 60, DV_CYAN_MID);
    tft.setTextColor(DV_CYAN, DV_PANEL); tft.setCursor(80, 126); tft.print(proto);

    tft.drawFastHLine(16, 140, 208, DV_CYAN_MID);
    tft.setTextColor(DV_PURPLE, DV_PANEL); tft.setCursor(16, 146); tft.print("ADDR");
    snprintf(buf, sizeof(buf), "0x%04lX", (unsigned long)addr);
    tft.setTextColor(DV_PINK, DV_PANEL); tft.setCursor(80, 146); tft.print(buf);

    tft.drawFastHLine(16, 160, 208, DV_CYAN_MID);
    tft.setTextColor(DV_PURPLE, DV_PANEL); tft.setCursor(16, 166); tft.print("CMD");
    snprintf(buf, sizeof(buf), "0x%04lX", (unsigned long)cmd);
    tft.setTextColor(DV_PINK, DV_PANEL); tft.setCursor(80, 166); tft.print(buf);
}

void drawIRSend(bool pulse) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PINK, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// IR Transmit");

    drawCenteredText("TRANSMITTING", 88, pulse ? DV_CYAN : DV_CYAN_DIM, 2);

    int cx = 120, cy = 168;
    tft.drawCircle(cx, cy, 20, DV_PINK);
    tft.drawCircle(cx, cy, 35, DV_PURPLE);
    tft.drawCircle(cx, cy, 50, DV_CYAN);
    if (pulse) tft.fillCircle(cx, cy, 6, DV_PINK);
}

void drawIRSavedList(const char **items, uint8_t count,
                     uint8_t sel, uint8_t offset) {
    drawMenu(items, count, sel, offset, "Saved IR Signals");
}

void drawFileList(const char *title, const char **items,
                  uint8_t count, uint8_t sel, uint8_t offset) {
    drawMenu(items, count, sel, offset, title);
}

void drawBrightness(int level) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Brightness");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", level);
    drawCenteredText(buf, 76, DV_CYAN, 3);

    int barW = (200 * level) / 255;
    tft.fillRect(20, 128, 200, 16, DV_BG2);
    tft.drawRect(20, 128, 200, 16, DV_CYAN_MID);
    tft.fillRect(20, 128, barW, 16, DV_CYAN);

    drawCenteredText("UP/DOWN to adjust", 158, DV_CYAN_DIM, 1);
    drawCenteredText("OK to confirm", 172, DV_CYAN_DIM, 1);
}

void drawAbout() {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// About IRUTESAM");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    drawCenteredText("IRUTESAM", 44, DV_CYAN, 2);
    drawCenteredText("Dark Venom", 68, DV_PURPLE, 1);
    drawCenteredText("v1.0", 82, DV_PURPLE, 1);

    tft.drawFastHLine(20, 96, 200, DV_CYAN_MID);

    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 108); tft.print("Board: ESP32 DevKit V1");
    tft.setCursor(6, 122); tft.print("Display: ILI9341 2.8in 240x320");
    tft.setCursor(6, 136); tft.print("NFC: PN532 I2C");
    tft.setCursor(6, 150); tft.print("IR: VS1838B RX / 38kHz TX");

    tft.setTextColor(DV_CYAN, DV_BG); tft.setCursor(6, 170); tft.print("WiFi: ");
    tft.setTextColor(wifiReady ? DV_GREEN : DV_RED, DV_BG); tft.print(wifiReady ? "READY" : "OFFLINE");

    tft.setTextColor(DV_CYAN, DV_BG); tft.setCursor(6, 184); tft.print("SD:   ");
    tft.setTextColor(sdAvailable ? DV_GREEN : DV_RED, DV_BG); tft.print(sdAvailable ? "MOUNTED" : "MISSING");

    tft.setTextColor(DV_CYAN, DV_BG); tft.setCursor(6, 198); tft.print("NFC:  ");
    tft.setTextColor(nfcAvailable ? DV_GREEN : DV_RED, DV_BG); tft.print(nfcAvailable ? "ONLINE" : "OFFLINE");

    tft.setTextColor(DV_CYAN, DV_BG); tft.setCursor(6, 212); tft.print("Touch:");
    tft.setTextColor(touchAvailable ? DV_GREEN : DV_AMBER, DV_BG); tft.print(touchAvailable ? " READY" : " OFFLINE");
}

void drawTouchCalCross(uint8_t step, int x, int y) {
    clearContentArea();
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Touch Calibration");

    char msg[24];
    snprintf(msg, sizeof(msg), "Tap corner %u of 4", step + 1);
    drawCenteredText(msg, 152, DV_CYAN_DIM, 1);

    tft.drawFastHLine(x - 12, y, 24, DV_CYAN);
    tft.drawFastVLine(x, y - 12, 24, DV_CYAN);
    tft.drawCircle(x, y, 6, DV_PINK);
}
