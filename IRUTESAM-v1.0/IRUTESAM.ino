// ════════════════════════════════════════════════════════════
// IRUTESAM v2.0 — Dark Venom
// FILE: IRUTESAM.ino
// BOARD: ESP32 DevKit V1
// DISPLAY: ILI9341 240x320 PORTRAIT
//
// Restructured with:
//   - New 5-item home menu (WiFi/NFC/IR/Saved Files/Settings)
//   - Navigation stack (NavManager) — proper goBack() everywhere
//   - WiFi module: scan, detail, security analysis, port scanner
//   - NFC & IR modules: unchanged logic, new nav integration
//   - Saved Files: IR Files / NFC Files sub-menus
// ════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

#include "config.h"
#include "nav_manager.h"
#include "touch_input.h"
#include "ir_module.h"
#include "nfc_module.h"
#include "sd_manager.h"
#include "tft_ui.h"
#include "wifi_module.h"

// ── Global flags ─────────────────────────────────────────────
bool wifiReady      = false;
bool sdAvailable    = false;
bool nfcAvailable   = false;
bool touchAvailable = false;
int  directRowIndex = -1;

extern TFT_eSPI tft;

// ════════════════════════════════════════════════════════════
// MENU DEFINITIONS
// ════════════════════════════════════════════════════════════
static const char *HOME_ITEMS[]     = { "Wi-Fi Tools", "NFC Tools", "IR Tools", "Saved Files", "Settings" };
static const uint8_t HOME_COUNT     = 5;

static const char *WIFI_ITEMS[]     = { "Scan Networks", "Security Analysis", "Network Detail", "Port Scanner", "Wi-Fi Info" };
static const uint8_t WIFI_COUNT     = 5;

static const char *NFC_ITEMS[]      = { "NFC Scan", "NFC Emulation", "Saved NFC Cards", "NFC Info" };
static const uint8_t NFC_COUNT      = 4;

static const char *IR_ITEMS[]       = { "IR Capture", "Send IR", "Saved IR Signals", "IR Info" };
static const uint8_t IR_COUNT       = 4;

static const char *FILES_ITEMS[]    = { "IR Files", "NFC Files" };
static const uint8_t FILES_COUNT    = 2;

static const char *SETTINGS_ITEMS[] = { "Brightness", "About" };
static const uint8_t SETTINGS_COUNT = 2;

// ════════════════════════════════════════════════════════════
// RUNTIME STATE
// ════════════════════════════════════════════════════════════

// ── WiFi ─────────────────────────────────────────────────────
static PortScanResult portResults[WIFI_PORT_COUNT];
static uint8_t        portCount        = 0;
static char           portScanIP[20]   = "192.168.1.1";
static bool           portScanDone     = false;
static bool           wifiScanning     = false;
static char           portScanIPInput[20] = "192.168.1.1";

// ── IR ───────────────────────────────────────────────────────
static char        irFileNames[IR_FILES_MAX][IR_NAME_MAX + 5];
static const char *irFilePtrs[IR_FILES_MAX];
static uint8_t     irFileCount  = 0;

static bool          irCapturing      = false;
static bool          replayPressed    = false;
static bool          replayContinuous = false;
static unsigned long replayPressStart = 0;
static unsigned long lastReplaySend   = 0;
#define REPLAY_LONGPRESS_MS  500
#define REPLAY_REPEAT_MS     180

// ── NFC ──────────────────────────────────────────────────────
static char        nfcFileNames[NFC_FILES_MAX][NFC_NAME_MAX + 5];
static const char *nfcFilePtrs[NFC_FILES_MAX];
static uint8_t     nfcFileCount = 0;

// ── Save dialog ──────────────────────────────────────────────
static bool dialogYes = true;

// ── Name keyboard ─────────────────────────────────────────────
#define NAME_MAX_LEN 12
static char    inputName[NAME_MAX_LEN + 1];
static uint8_t inputLen  = 0;
static bool    inputForIR = true;   // true = saving IR, false = saving NFC

// ── Brightness ───────────────────────────────────────────────
static int brightness = 180;

// ── Animation ────────────────────────────────────────────────
static unsigned long lastPulse  = 0;
static bool          pulseState = false;
static unsigned long lastScanAnim = 0;
static uint8_t       scanStep    = 0;

// ── NFC emulate screen ───────────────────────────────────────
static unsigned long lastNFCPulse = 0;
static bool          nfcPulse     = false;

// ════════════════════════════════════════════════════════════
// KEYBOARD
// ════════════════════════════════════════════════════════════
static const char KB_KEYS[] =
    "ABCDEFGHIJ"
    "KLMNOPQRST"
    "UVWXYZ0123"
    "456789";
#define KB_COLS    10
#define KB_KEY_W   21
#define KB_KEY_H   24
#define KB_START_X  4
#define KB_START_Y 78
#define KB_GAP      2

static void drawKeyboard() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Enter Signal Name");

    tft.fillRoundRect(8, 36, 224, 22, 3, DV_PANEL);
    tft.drawRoundRect(8, 36, 224, 22, 3, DV_CYAN_MID);

    char displayBuf[NAME_MAX_LEN + 2];
    strncpy(displayBuf, inputName, inputLen);
    displayBuf[inputLen]     = '_';
    displayBuf[inputLen + 1] = '\0';

    tft.setTextSize(1);
    tft.setTextColor(DV_CYAN, DV_PANEL);
    tft.setCursor(12, 43);
    tft.print(displayBuf);

    char lenBuf[8];
    snprintf(lenBuf, sizeof(lenBuf), "%u/12", inputLen);
    tft.setCursor(198, 43);
    tft.print(lenBuf);

    const uint8_t totalKeys = strlen(KB_KEYS);
    for (uint8_t i = 0; i < totalKeys; i++) {
        uint8_t row = i / KB_COLS;
        uint8_t col = i % KB_COLS;
        int kx = KB_START_X + col * (KB_KEY_W + KB_GAP);
        int ky = KB_START_Y + row * (KB_KEY_H + KB_GAP);
        tft.fillRoundRect(kx, ky, KB_KEY_W, KB_KEY_H, 2, DV_PANEL);
        tft.drawRoundRect(kx, ky, KB_KEY_W, KB_KEY_H, 2, DV_CYAN_MID);
        tft.setTextColor(DV_CYAN, DV_PANEL);
        tft.setTextSize(1);
        tft.setCursor(kx + 6, ky + 8);
        char c[2] = { KB_KEYS[i], '\0' };
        tft.print(c);
    }

    int delY  = KB_START_Y + 4 * (KB_KEY_H + KB_GAP);
    int saveX = 64, saveW = 172;

    tft.fillRoundRect(4, delY, 48, 24, 2, DV_RED_FILL);
    tft.drawRoundRect(4, delY, 48, 24, 2, DV_RED);
    tft.setTextColor(DV_CYAN, DV_RED_FILL);
    tft.setCursor(16, delY + 8);
    tft.print("DEL");

    tft.fillRoundRect(saveX, delY, saveW, 24, 2, DV_HILIGHT);
    tft.drawRoundRect(saveX, delY, saveW, 24, 2, DV_GREEN);
    tft.setTextColor(DV_CYAN, DV_HILIGHT);
    tft.setCursor(saveX + 72, delY + 8);
    tft.print("SAVE");
}

static void handleKeyboardTouch(int tx, int ty) {
    const uint8_t totalKeys = strlen(KB_KEYS);

    for (uint8_t i = 0; i < totalKeys; i++) {
        uint8_t row = i / KB_COLS;
        uint8_t col = i % KB_COLS;
        int kx = KB_START_X + col * (KB_KEY_W + KB_GAP);
        int ky = KB_START_Y + row * (KB_KEY_H + KB_GAP);

        if (tx >= kx && tx <= kx + KB_KEY_W &&
            ty >= ky && ty <= ky + KB_KEY_H) {
            if (inputLen < NAME_MAX_LEN) {
                inputName[inputLen++] = KB_KEYS[i];
                inputName[inputLen]   = '\0';
                tft.fillRoundRect(kx, ky, KB_KEY_W, KB_KEY_H, 2, DV_HILIGHT);
                tft.drawRoundRect(kx, ky, KB_KEY_W, KB_KEY_H, 2, DV_CYAN);
                tft.setTextColor(DV_CYAN, DV_HILIGHT);
                tft.setTextSize(1);
                tft.setCursor(kx + 6, ky + 8);
                char c[2] = { KB_KEYS[i], '\0' };
                tft.print(c);
                delay(60);
            }
            drawKeyboard();
            return;
        }
    }

    int delY  = KB_START_Y + 4 * (KB_KEY_H + KB_GAP);
    int saveX = 64, saveW = 172;

    if (tx >= 4 && tx <= 52 && ty >= delY && ty <= delY + 24) {
        if (inputLen > 0) { inputLen--; inputName[inputLen] = '\0'; }
        drawKeyboard();
        return;
    }

    if (tx >= saveX && tx <= saveX + saveW && ty >= delY && ty <= delY + 24) {
        if (inputLen == 0) return;

        bool saved = false;
        if (inputForIR) {
            strncpy(capturedIR.name, inputName, IR_NAME_MAX);
            capturedIR.name[IR_NAME_MAX] = '\0';
            saved = saveIRToSD(capturedIR);
        } else {
            strncpy(capturedNFC.name, inputName, NFC_NAME_MAX);
            capturedNFC.name[NFC_NAME_MAX] = '\0';
            saved = saveNFCToSD(capturedNFC);
        }

        tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
        drawCenteredText(saved ? "SAVED!" : "SAVE FAILED", 140, DV_CYAN, 2);
        delay(600);
        nav.goTo(SCR_HOME);
    }
}

// ════════════════════════════════════════════════════════════
// SCROLL MENU HELPER
// Handles TOUCH_UP / TOUCH_DOWN / TOUCH_RAW / TOUCH_OPEN for
// any scrollable list. Returns true when an item is opened.
// ════════════════════════════════════════════════════════════
static bool handleMenuTouch(TouchEvent tevt,
                             int tx, int ty,
                             const char **items, uint8_t count,
                             void (*drawFn)(const char**, uint8_t, uint8_t, uint8_t, const char*),
                             const char *title) {
    uint8_t sel    = nav.sel();
    uint8_t offset = nav.offset();

    if (tevt == TOUCH_UP) {
        if (sel > 0) {
            uint8_t oldSel = sel;
            sel--;
            if (sel < offset) {
                offset = sel;
                drawFn(items, count, sel, offset, title);
            } else {
                int oldRow = MENU_START_Y + (oldSel - offset) * MENU_ROW_H;
                int newRow = MENU_START_Y + (sel    - offset) * MENU_ROW_H;
                drawMenuRowPartial(items[oldSel], oldRow, false);
                drawMenuRowPartial(items[sel],    newRow, true);
            }
            nav.setSel(sel);
            nav.setOffset(offset);
        }
        return false;
    }

    if (tevt == TOUCH_DOWN) {
        if (sel < count - 1) {
            uint8_t oldSel = sel;
            sel++;
            if (sel >= offset + MENU_MAX_ROWS) {
                offset = sel - MENU_MAX_ROWS + 1;
                drawFn(items, count, sel, offset, title);
            } else {
                int oldRow = MENU_START_Y + (oldSel - offset) * MENU_ROW_H;
                int newRow = MENU_START_Y + (sel    - offset) * MENU_ROW_H;
                drawMenuRowPartial(items[oldSel], oldRow, false);
                drawMenuRowPartial(items[sel],    newRow, true);
            }
            nav.setSel(sel);
            nav.setOffset(offset);
        }
        return false;
    }

    if (tevt == TOUCH_RAW) {
        // Direct row tap
        for (uint8_t r = 0; r < MENU_MAX_ROWS; r++) {
            uint8_t idx = offset + r;
            if (idx >= count) break;
            int rowY = MENU_START_Y + r * MENU_ROW_H;
            if (ty >= rowY && ty < rowY + MENU_ROW_H) {
                if (sel != idx) {
                    if (sel >= offset && sel < offset + MENU_MAX_ROWS) {
                        int oldRowY = MENU_START_Y + (sel - offset) * MENU_ROW_H;
                        drawMenuRowPartial(items[sel], oldRowY, false);
                    }
                    sel = idx;
                    nav.setSel(sel);
                    drawMenuRowPartial(items[sel], rowY, true);
                }
                // single tap just selects, no open
                return false;
            }
        }
        return false;
    }

    if (tevt == TOUCH_OPEN) {
        return true;  // caller opens selected item
    }

    return false;
}

// ── Inline drawMenu wrapper matching the helper signature ────
static void drawMenuWrapper(const char **items, uint8_t count,
                             uint8_t sel, uint8_t offset, const char *title) {
    drawMenu(items, count, sel, offset, title);
}

// ════════════════════════════════════════════════════════════
// IR REPLAY helpers
// ════════════════════════════════════════════════════════════
static void drawIRReplayScreen() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_PINK, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// IR Replay");

    if (!selectedIR.hasSignal) {
        drawCenteredText("No signal selected",     96, DV_CYAN,     1);
        drawCenteredText("Open a saved signal first", 112, DV_CYAN_DIM, 1);
        return;
    }

    drawCenteredText(selectedIR.name, 40, DV_CYAN, 2);

    char infoBuf[32];
    snprintf(infoBuf, sizeof(infoBuf), "%s  %u-bit",
             typeToString(selectedIR.protocol).c_str(), selectedIR.bits);
    drawCenteredText(infoBuf, 62, DV_CYAN_DIM, 1);

    tft.fillRoundRect(8, 80, 224, 28, 3, DV_PANEL);
    tft.drawRoundRect(8, 80, 224, 28, 3, DV_CYAN_MID);

    if (selectedIR.rawLen > 0) {
        uint32_t maxV = 1;
        for (uint16_t i = 0; i < selectedIR.rawLen; i++)
            if (selectedIR.rawData[i] > maxV) maxV = selectedIR.rawData[i];
        int xPos = 10, drawW = 220;
        int pxPerSample = max(1, drawW / (int)selectedIR.rawLen);
        for (uint16_t i = 0; i < selectedIR.rawLen && xPos < 230; i++) {
            int w = max(1, (int)((selectedIR.rawData[i] * pxPerSample * 4) / maxV));
            if (xPos + w > 230) w = 230 - xPos;
            int barY = (i % 2 == 0) ? 84 : 96;
            tft.fillRect(xPos, barY, w, 10, (i % 2 == 0) ? DV_CYAN : DV_CYAN_MID);
            xPos += w;
        }
    }

    int bx = 25, by = 140, bw = 190, bh = 44;
    tft.fillRoundRect(bx, by, bw, bh, 4, DV_BG2);
    tft.drawRoundRect(bx, by, bw, bh, 4, DV_CYAN_MID);
    tft.setTextSize(2);
    tft.setTextColor(DV_CYAN, DV_BG2);
    tft.setCursor(bx + 42, by + 12);
    tft.print("REPLAY");

    tft.fillRect(0, 248, SCREEN_W, 16, DV_BG);
}

static void updateReplayButtonPulse(bool glow) {
    int bx = 25, by = 140, bw = 190, bh = 44;
    tft.fillRoundRect(bx, by, bw, bh, 4, glow ? DV_HILIGHT : DV_BG2);
    tft.drawRoundRect(bx, by, bw, bh, 4, glow ? DV_CYAN : DV_CYAN_MID);
    tft.setTextSize(2);
    tft.setTextColor(glow ? DV_CYAN : DV_CYAN_DIM, glow ? DV_HILIGHT : DV_BG2);
    tft.setCursor(bx + 42, by + 12);
    tft.print("REPLAY");
}

static void drawReplayStatusText(const char *msg) {
    tft.fillRect(0, 248, SCREEN_W, 16, DV_BG);
    drawCenteredText(msg, 252, DV_CYAN_DIM, 1);
}

static bool replayButtonHit(int tx, int ty) {
    return (tx >= 19 && tx <= 221 && ty >= 134 && ty <= 190);
}

// ── Brightness bar update ────────────────────────────────────
static void drawBrightnessBarOnly(int level) {
    int barW = (200 * level) / 255;
    tft.fillRect(40, 76, 160, 24, DV_BG);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", level);
    drawCenteredText(buf, 76, DV_CYAN, 3);
    tft.fillRect(20, 128, 200, 16, DV_BG2);
    tft.drawRect(20, 128, 200, 16, DV_CYAN_MID);
    tft.fillRect(20, 128, barW, 16, DV_CYAN);
}

// ════════════════════════════════════════════════════════════
// WIFI SCREEN DRAWS (local to .ino for portability)
// ════════════════════════════════════════════════════════════
static int wifiRSSI[WIFI_MAX_NETWORKS];
static int wifiSec[WIFI_MAX_NETWORKS];
static const char *wifiSSIDPtrs[WIFI_MAX_NETWORKS];

static void buildWifiPtrs() {
    for (uint8_t i = 0; i < wifiNetCount; i++) {
        wifiSSIDPtrs[i] = wifiNets[i].ssid;
        wifiRSSI[i]     = wifiNets[i].rssi;
        wifiSec[i]      = wifiNets[i].secType;
    }
}

static void drawWifiInfoScreen() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Wi-Fi Information");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 42);  tft.print("This tool scans and analyses");
    tft.setCursor(6, 56);  tft.print("nearby Wi-Fi networks.");
    tft.setCursor(6, 78);  tft.print("Features:");
    tft.setTextColor(DV_CYAN, DV_BG);
    tft.setCursor(6, 94);  tft.print("> Network scanner (SSID/BSSID)");
    tft.setCursor(6, 108); tft.print("> Signal strength & encryption");
    tft.setCursor(6, 122); tft.print("> Security analysis & risk score");
    tft.setCursor(6, 136); tft.print("> Local network port scanner");
    tft.drawFastHLine(0, 152, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_AMBER, DV_BG);
    tft.setCursor(6, 162); tft.print("For educational/diagnostic use");
    tft.setCursor(6, 176); tft.print("on networks you own/authorise.");
}

static void drawPortScanScreen(bool scanning) {
    if (scanning) {
        tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
        tft.setTextSize(1);
        tft.setTextColor(DV_PURPLE, DV_BG);
        tft.setCursor(6, 20);
        tft.print("// Port Scanner");
        tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);
        tft.setTextColor(DV_CYAN_DIM, DV_BG);
        tft.setCursor(6, 50); tft.print("Target: ");
        tft.setTextColor(DV_CYAN, DV_BG);
        tft.print(portScanIP);
        drawCenteredText("SCANNING...", 130, DV_AMBER, 2);
        return;
    }

    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Port Scanner");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);

    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 40); tft.print("Target: ");
    tft.setTextColor(DV_CYAN, DV_BG);
    tft.print(portScanIP);

    int yPos = 58;
    for (uint8_t i = 0; i < portCount; i++) {
        bool o = portResults[i].open;
        tft.setTextColor(DV_CYAN_DIM, DV_BG);
        tft.setCursor(6, yPos);
        char buf[24];
        snprintf(buf, sizeof(buf), ":%u", portResults[i].port);
        tft.print(buf);
        tft.setCursor(70, yPos);
        tft.setTextColor(o ? DV_GREEN : DV_RED, DV_BG);
        tft.print(o ? "OPEN" : "closed");
        yPos += 16;
    }
}

static void drawNFCInfoScreen() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// NFC Information");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 42);  tft.print("Hardware: PN532 UART");
    tft.setCursor(6, 58);  tft.print("Supports: MIFARE Classic");
    tft.setCursor(6, 74);  tft.print("UID read: 4 or 7 bytes");
    tft.setCursor(6, 90);  tft.print("Emulation: 4-byte UID only");
    tft.drawFastHLine(0, 106, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_CYAN, DV_BG);
    tft.setCursor(6, 118); tft.print("Status: ");
    tft.setTextColor(nfcAvailable ? DV_GREEN : DV_RED, DV_BG);
    tft.print(nfcAvailable ? "ONLINE" : "OFFLINE");
    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 134); tft.print("UART RX: GPIO");
    tft.print(PN532_RX_PIN);
    tft.print("  TX: GPIO");
    tft.print(PN532_TX_PIN);
}

static void drawIRInfoScreen() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// IR Information");
    tft.drawFastHLine(0, 30, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 42);  tft.print("RX: VS1838B  GPIO"); tft.print(IR_RX_PIN);
    tft.setCursor(6, 58);  tft.print("TX: 38kHz LED  GPIO"); tft.print(IR_TX_PIN);
    tft.setCursor(6, 74);  tft.print("Protocols: NEC/SONY/RC5/RC6");
    tft.setCursor(6, 90);  tft.print("           SAMSUNG/LG/PANASONIC");
    tft.setCursor(6, 106); tft.print("           DENON/JVC/SHARP + raw");
    tft.drawFastHLine(0, 122, SCREEN_W, DV_CYAN_MID);
    tft.setTextColor(DV_CYAN_DIM, DV_BG);
    tft.setCursor(6, 134); tft.print("Capture buffer: ");
    tft.print(IR_MAX_RAW);
    tft.print(" samples");
}

// ════════════════════════════════════════════════════════════
// SETUP
// ════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);

    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);

    pinMode(TFT_CS_PIN,   OUTPUT);
    pinMode(TOUCH_CS_PIN, OUTPUT);
    pinMode(SD_CS_PIN,    OUTPUT);
    digitalWrite(TFT_CS_PIN,   HIGH);
    digitalWrite(TOUCH_CS_PIN, HIGH);
    digitalWrite(SD_CS_PIN,    HIGH);

    tftInit();
    tft.fillScreen(DV_BG);
    tft.setTextColor(DV_CYAN, DV_BG);

    touchInit();
    drawBootScreen();

    sdAvailable = sdInit();
    irModuleInit();
    nfcModuleInit();
    wifiModuleInit();

    nav.goTo(SCR_HOME);
}

// ════════════════════════════════════════════════════════════
// LOOP
// ════════════════════════════════════════════════════════════
void loop() {
    TouchEvent tevt  = touchPoll();
    bool       touched = isTouched();
    int        tx    = getLastTouchX();
    int        ty    = getLastTouchY();

    if (millis() - lastPulse > 380) {
        lastPulse  = millis();
        pulseState = !pulseState;
    }

    bool newScr = nav.isNewScreen();
    if (newScr) nav.clearNew();

    // ── Universal footer BACK ─────────────────────────────────
    // Any tap in the footer area (except home) goes back.
    // We check this before the per-screen switch.
    if (tevt != TOUCH_NONE && ty >= FOOTER_Y) {
        Screen cur = nav.current();
        if (cur != SCR_HOME && cur != SCR_BOOT &&
            cur != SCR_IR_NAME_INPUT &&
            cur != SCR_NFC_NAME_INPUT &&
            cur != SCR_NFC_SAVE_DIALOG &&
            cur != SCR_IR_SAVE_DIALOG) {
            // Special cleanup before back
            if (cur == SCR_NFC_EMULATE || cur == SCR_FILES_NFC_EMULATE)
                nfcStopEmulate();
            if (cur == SCR_IR_CAPTURE)
                irStopCapture();
            nav.goBack();
            return;
        }
    }

    switch (nav.current()) {

    // ══════════════════════════════════════════════════════════
    case SCR_HOME: {
        if (newScr) {
            drawStatusBar("IRUTESAM", DV_CYAN);
            drawMenu(HOME_ITEMS, HOME_COUNT, nav.sel(), nav.offset(), "MAIN MENU");
            drawFooter("TAP", "select", "HOLD=open");
        }
        if (tevt == TOUCH_NONE) break;

        bool opened = handleMenuTouch(tevt, tx, ty,
                                      HOME_ITEMS, HOME_COUNT,
                                      drawMenuWrapper, "MAIN MENU");
        if (opened) {
            switch (nav.sel()) {
                case 0: nav.goTo(SCR_WIFI_MENU);     break;
                case 1: nav.goTo(SCR_NFC_MENU);      break;
                case 2: nav.goTo(SCR_IR_MENU);       break;
                case 3: nav.goTo(SCR_FILES_MENU);    break;
                case 4: nav.goTo(SCR_SETTINGS_MENU); break;
            }
        }
        break;
    }

    // ══════════════════════════════════════════════════════════
    // WIFI
    // ══════════════════════════════════════════════════════════
    case SCR_WIFI_MENU: {
        if (newScr) {
            drawStatusBar("Wi-Fi Tools", DV_CYAN_MID);
            drawMenu(WIFI_ITEMS, WIFI_COUNT, nav.sel(), nav.offset(), "WI-FI TOOLS");
            drawFooter("TAP", "select", "HOLD=open");
        }
        if (tevt == TOUCH_NONE) break;
        bool opened = handleMenuTouch(tevt, tx, ty, WIFI_ITEMS, WIFI_COUNT, drawMenuWrapper, "WI-FI TOOLS");
        if (opened) {
            switch (nav.sel()) {
                case 0: nav.goTo(SCR_WIFI_SCAN);     break;
                case 1: nav.goTo(SCR_WIFI_SECURITY); break;
                case 2: nav.goTo(SCR_WIFI_DETAIL);   break;
                case 3: nav.goTo(SCR_WIFI_PORT_SCAN);break;
                case 4: nav.goTo(SCR_WIFI_INFO);     break;
            }
        }
        break;
    }

    case SCR_WIFI_SCAN: {
        if (newScr) {
            drawStatusBar("Scan Networks", DV_CYAN_MID);
            tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
            tft.setTextSize(1);
            tft.setTextColor(DV_PURPLE, DV_BG);
            tft.setCursor(6, 20);
            tft.print("// Scanning Wi-Fi...");
            drawCenteredText("Please wait...", 140, DV_CYAN_DIM, 1);
            drawFooter("", "", "");

            wifiNetCount = wifiScan(wifiNets, WIFI_MAX_NETWORKS);
            wifiReady    = (wifiNetCount > 0);
            buildWifiPtrs();

            if (wifiNetCount == 0) {
                drawCenteredText("No networks found", 140, DV_CYAN_DIM, 1);
                drawFooter("TAP", "footer", "BACK");
                break;
            }
            drawWifiList(wifiSSIDPtrs, wifiRSSI, wifiSec, wifiNetCount,
                         nav.sel(), nav.offset());
            drawFooter("TOP/BOT", "scroll", "HOLD=open");
        }

        if (wifiNetCount == 0) break;

        if (tevt == TOUCH_NONE) break;

        if (tevt == TOUCH_UP) {
            uint8_t sel = nav.sel(), off = nav.offset();
            if (sel > 0) {
                uint8_t old = sel--;
                if (sel < off) { off = sel; drawWifiList(wifiSSIDPtrs, wifiRSSI, wifiSec, wifiNetCount, sel, off); }
                else {
                    int oy = WIFI_LIST_START_Y + (old - off) * WIFI_LIST_ROW_H;
                    int ny = WIFI_LIST_START_Y + (sel - off) * WIFI_LIST_ROW_H;
                    // redraw both rows
                    drawWifiList(wifiSSIDPtrs, wifiRSSI, wifiSec, wifiNetCount, sel, off);
                }
                nav.setSel(sel); nav.setOffset(off);
            }
            break;
        }

        if (tevt == TOUCH_DOWN) {
            uint8_t sel = nav.sel(), off = nav.offset();
            if (sel < wifiNetCount - 1) {
                uint8_t old = sel++;
                if (sel >= off + WIFI_LIST_MAX_VIS) { off = sel - WIFI_LIST_MAX_VIS + 1; }
                drawWifiList(wifiSSIDPtrs, wifiRSSI, wifiSec, wifiNetCount, sel, off);
                nav.setSel(sel); nav.setOffset(off);
            }
            break;
        }

        if (tevt == TOUCH_OPEN) {
            memcpy(&selectedNet, &wifiNets[nav.sel()], sizeof(WiFiNetwork));
            nav.goTo(SCR_WIFI_DETAIL);
            break;
        }

        if (tevt == TOUCH_RAW) {
            // direct row tap — select
            uint8_t off = nav.offset();
            for (uint8_t r = 0; r < WIFI_LIST_MAX_VIS; r++) {
                uint8_t idx = off + r;
                if (idx >= wifiNetCount) break;
                int rowY = WIFI_LIST_START_Y + r * WIFI_LIST_ROW_H;
                if (ty >= rowY && ty < rowY + WIFI_LIST_ROW_H) {
                    nav.setSel(idx);
                    drawWifiList(wifiSSIDPtrs, wifiRSSI, wifiSec, wifiNetCount, idx, off);
                    break;
                }
            }
        }
        break;
    }

    case SCR_WIFI_DETAIL: {
        if (newScr) {
            drawStatusBar("Network Detail", DV_CYAN_MID);
            drawWifiDetail(selectedNet.ssid, selectedNet.bssid,
                           selectedNet.rssi, selectedNet.channel,
                           selectedNet.secType);
            drawFooter("BACK", "return", "");
        }
        break;
    }

    case SCR_WIFI_PORT_SCAN: {
        if (newScr) {
            drawStatusBar("Port Scanner", DV_CYAN_MID);
            // Use router IP from last scan if available
            if (wifiNetCount > 0) {
                // Default to common gateway
                strncpy(portScanIP, "192.168.1.1", sizeof(portScanIP));
            }
            drawPortScanScreen(true);
            drawFooter("", "scanning", "");
            portCount   = wifiPortScan(portScanIP, portResults, WIFI_PORT_COUNT);
            portScanDone = true;
            drawPortScanScreen(false);
            drawFooter("BACK", "return", "");
        }
        break;
    }

    case SCR_WIFI_SECURITY: {
        if (newScr) {
            drawStatusBar("Security Analysis", DV_CYAN_MID);
            if (wifiNetCount == 0) {
                tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
                drawCenteredText("No scan data.", 130, DV_CYAN_DIM, 1);
                drawCenteredText("Run Scan Networks first.", 146, DV_CYAN_DIM, 1);
            } else {
                drawSecurityAnalysis(wifiSSIDPtrs, wifiSec, wifiNetCount,
                                     nav.sel(), nav.offset());
            }
            drawFooter("BACK", "return", "");
        }
        break;
    }

    case SCR_WIFI_INFO: {
        if (newScr) {
            drawStatusBar("Wi-Fi Info", DV_CYAN_MID);
            drawWifiInfoScreen();
            drawFooter("BACK", "return", "");
        }
        break;
    }

    // ══════════════════════════════════════════════════════════
    // NFC
    // ══════════════════════════════════════════════════════════
    case SCR_NFC_MENU: {
        if (newScr) {
            drawStatusBar("NFC Tools", DV_PURPLE);
            drawMenu(NFC_ITEMS, NFC_COUNT, nav.sel(), nav.offset(), "NFC TOOLS");
            drawFooter("TAP", "select", "HOLD=open");
        }
        if (tevt == TOUCH_NONE) break;
        bool opened = handleMenuTouch(tevt, tx, ty, NFC_ITEMS, NFC_COUNT, drawMenuWrapper, "NFC TOOLS");
        if (opened) {
            switch (nav.sel()) {
                case 0: nav.goTo(SCR_NFC_SCAN);       break;
                case 1: nav.goTo(SCR_NFC_EMULATE);    break;
                case 2:
                    nfcFileCount = listNFCFiles(nfcFileNames, NFC_FILES_MAX);
                    for (uint8_t i = 0; i < nfcFileCount; i++) nfcFilePtrs[i] = nfcFileNames[i];
                    nav.goTo(SCR_NFC_SAVED_LIST);
                    break;
                case 3: nav.goTo(SCR_NFC_INFO); break;
            }
        }
        break;
    }

    case SCR_NFC_SCAN: {
        if (newScr) {
            scanStep    = 0;
            lastScanAnim = 0;
            drawStatusBar("NFC Scan", DV_PURPLE);
            drawNFCScan(scanStep);
            drawFooter("SCAN", "tag", "BACK");
        }

        if (millis() - lastScanAnim > 180) {
            lastScanAnim = millis();
            scanStep++;
            drawNFCScan(scanStep);
        }

        if (nfcPollCard(capturedNFC)) {
            drawStatusBar("NFC Scan", DV_PURPLE);
            drawNFCScan(scanStep);
            drawNFCResult(capturedNFC.uidStr, capturedNFC.uidLen);
            delay(700);
            dialogYes = true;
            nav.goTo(SCR_NFC_SAVE_DIALOG);
        }
        break;
    }

    case SCR_NFC_SAVE_DIALOG: {
        if (newScr) {
            drawStatusBar("Save NFC?", DV_GREEN);
            drawNFCScan(0);
            drawNFCResult(capturedNFC.uidStr, capturedNFC.uidLen);
            drawDialog("Save Card?", "Store UID to SD?", "", dialogYes);
            drawFooter("YES", "save", "NO");
        }
        if (touched) {
            bool changed = checkDialogTouch(dialogYes);
            if (changed) {
                if (dialogYes) {
                    memset(inputName, 0, sizeof(inputName));
                    inputLen  = 0;
                    inputForIR = false;
                    nav.goTo(SCR_NFC_NAME_INPUT);
                } else {
                    nav.goTo(SCR_NFC_SCAN);
                }
            }
        }
        break;
    }

    case SCR_NFC_NAME_INPUT: {
        if (newScr) {
            drawStatusBar("Save NFC", DV_GREEN);
            drawKeyboard();
            drawFooter("TYPE", "name", "SAVE");
        }
        if (touched) handleKeyboardTouch(tx, ty);
        break;
    }

    case SCR_NFC_EMULATE: {
        // Quick-emulate: needs a saved card selected first.
        // If no selectedNFC, prompt user to pick from saved list.
        if (newScr) {
            if (!selectedNFC.hasCard) {
                // Load first saved NFC card if any
                nfcFileCount = listNFCFiles(nfcFileNames, NFC_FILES_MAX);
                if (nfcFileCount > 0) {
                    loadNFCFromSD(nfcFileNames[0], selectedNFC);
                }
            }

            nfcPulse     = false;
            lastNFCPulse = 0;
            drawStatusBar("NFC Emulate", DV_PURPLE);
            drawNFCEmulate(selectedNFC.hasCard ? selectedNFC.uidStr : "NO CARD", nfcPulse);
            if (selectedNFC.hasCard)
                drawCenteredText(selectedNFC.name, 34, DV_CYAN, 2);

            drawFooter("EMULATE", "active", "BACK");

            if (!selectedNFC.hasCard) {
                drawCenteredText("No card selected!", 200, DV_RED, 1);
                drawCenteredText("Use Saved NFC Cards first", 216, DV_CYAN_DIM, 1);
                break;
            }

            if (!nfcStartEmulate(selectedNFC)) {
                drawCenteredText("4-byte UID only!", 200, DV_RED, 1);
                delay(1200);
                nav.goBack();
                break;
            }
        }

        if (!selectedNFC.hasCard) break;

        if (millis() - lastNFCPulse > 350) {
            lastNFCPulse = millis();
            nfcPulse     = !nfcPulse;
            updateNFCEmulatePulse(nfcPulse);
        }

        if (nfcEmulateTask()) {
            updateNFCEmulatePulse(true);
            delay(80);
            updateNFCEmulatePulse(nfcPulse);
        }
        break;
    }

    case SCR_NFC_SAVED_LIST: {
        if (newScr) {
            drawStatusBar("Saved NFC", DV_PURPLE);
            if (nfcFileCount == 0) {
                tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
                drawCenteredText("No saved NFC cards", 140, DV_CYAN_DIM, 1);
                drawCenteredText("Scan a card first", 156, DV_CYAN_DIM, 1);
            } else {
                drawNFCSavedList(nfcFilePtrs, nfcFileCount, nav.sel(), nav.offset());
            }
            drawFooter("TOP/BOT", "scroll", "HOLD=open");
        }
        if (tevt == TOUCH_NONE || nfcFileCount == 0) break;
        bool opened = handleMenuTouch(tevt, tx, ty, nfcFilePtrs, nfcFileCount, drawMenuWrapper, "Saved NFC Cards");
        if (opened) {
            bool loaded = loadNFCFromSD(nfcFileNames[nav.sel()], selectedNFC);
            if (loaded) {
                nfcPulse = false; lastNFCPulse = 0;
                nav.goTo(SCR_FILES_NFC_EMULATE);
            } else {
                drawCenteredText("Load failed!", FOOTER_Y - 16, DV_RED, 1);
            }
        }
        break;
    }

    case SCR_NFC_INFO: {
        if (newScr) {
            drawStatusBar("NFC Info", DV_PURPLE);
            drawNFCInfoScreen();
            drawFooter("BACK", "return", "");
        }
        break;
    }

    // ══════════════════════════════════════════════════════════
    // IR
    // ══════════════════════════════════════════════════════════
    case SCR_IR_MENU: {
        if (newScr) {
            drawStatusBar("IR Tools", DV_PINK);
            drawMenu(IR_ITEMS, IR_COUNT, nav.sel(), nav.offset(), "IR TOOLS");
            drawFooter("TAP", "select", "HOLD=open");
        }
        if (tevt == TOUCH_NONE) break;
        bool opened = handleMenuTouch(tevt, tx, ty, IR_ITEMS, IR_COUNT, drawMenuWrapper, "IR TOOLS");
        if (opened) {
            switch (nav.sel()) {
                case 0:
                    irStartCapture();
                    nav.goTo(SCR_IR_CAPTURE);
                    break;
                case 1:
                    if (!selectedIR.hasSignal) {
                        irFileCount = listIRFiles(irFileNames, IR_FILES_MAX);
                        for (uint8_t i = 0; i < irFileCount; i++) irFilePtrs[i] = irFileNames[i];
                        nav.goTo(SCR_IR_SAVED_LIST);
                    } else {
                        nav.goTo(SCR_IR_SEND);
                    }
                    break;
                case 2:
                    irFileCount = listIRFiles(irFileNames, IR_FILES_MAX);
                    for (uint8_t i = 0; i < irFileCount; i++) irFilePtrs[i] = irFileNames[i];
                    nav.goTo(SCR_IR_SAVED_LIST);
                    break;
                case 3:
                    nav.goTo(SCR_IR_INFO);
                    break;
            }
        }
        break;
    }

    case SCR_IR_CAPTURE: {
        if (newScr) {
            drawStatusBar("IR Capture", DV_PINK);
            drawIRCapture(capturedIR.rawData, capturedIR.rawLen);
            drawFooter("POINT", "remote at RX", "BACK");
        }

        if (irPollCapture(capturedIR)) {
            drawIRCapture(capturedIR.rawData, capturedIR.rawLen);
            drawIRSignalInfo(typeToString(capturedIR.protocol).c_str(),
                             (uint32_t)(capturedIR.value >> 16),
                             (uint32_t)(capturedIR.value & 0xFFFF));
            delay(400);
            dialogYes = true;
            irStopCapture();
            nav.goTo(SCR_IR_SAVE_DIALOG);
        }
        break;
    }

    case SCR_IR_SAVE_DIALOG: {
        if (newScr) {
            drawStatusBar("Save IR?", DV_GREEN);
            drawIRCapture(capturedIR.rawData, capturedIR.rawLen);
            drawDialog("Save Signal?", "Store to SD card?",
                       capturedIR.hasSignal ? "Signal captured" : "", dialogYes);
            drawFooter("YES", "save", "NO");
        }
        if (touched) {
            bool changed = checkDialogTouch(dialogYes);
            if (changed) {
                if (dialogYes) {
                    memset(inputName, 0, sizeof(inputName));
                    inputLen   = 0;
                    inputForIR = true;
                    nav.goTo(SCR_IR_NAME_INPUT);
                } else {
                    irStartCapture();
                    nav.goTo(SCR_IR_CAPTURE);
                }
            }
        }
        break;
    }

    case SCR_IR_NAME_INPUT: {
        if (newScr) {
            drawStatusBar("Save IR", DV_GREEN);
            drawKeyboard();
            drawFooter("TYPE", "name", "SAVE");
        }
        if (touched) handleKeyboardTouch(tx, ty);
        break;
    }

    case SCR_IR_SEND: {
        static bool lastReplayPulse = false;

        if (newScr) {
            replayPressed    = false;
            replayContinuous = false;
            lastReplayPulse  = false;
            drawStatusBar("Send IR", DV_PINK);
            drawIRReplayScreen();
            drawReplayStatusText("");
            drawFooter("TAP", "replay", "BACK");
        }

        if (selectedIR.hasSignal && pulseState != lastReplayPulse) {
            lastReplayPulse = pulseState;
            updateReplayButtonPulse(pulseState);
        }

        if (!touched) {
            if (replayPressed || replayContinuous) {
                replayPressed    = false;
                replayContinuous = false;
                drawReplayStatusText("");
                updateReplayButtonPulse(false);
            }
            break;
        }

        if (selectedIR.hasSignal && replayButtonHit(tx, ty)) {
            unsigned long now = millis();
            if (!replayPressed) {
                replayPressed    = true;
                replayContinuous = false;
                replayPressStart = now;
                lastReplaySend   = now;
                updateReplayButtonPulse(true);
                bool ok = irSendSignal(selectedIR);
                drawReplayStatusText(ok ? "Sent!" : "Failed!");
                touchClearState();
            } else {
                if (!replayContinuous && (now - replayPressStart >= REPLAY_LONGPRESS_MS)) {
                    replayContinuous = true;
                    lastReplaySend   = now;
                    drawReplayStatusText("Continuous...");
                }
                if (replayContinuous && (now - lastReplaySend >= REPLAY_REPEAT_MS)) {
                    lastReplaySend = now;
                    irSendSignal(selectedIR);
                }
            }
        } else {
            if (replayPressed || replayContinuous) {
                replayPressed    = false;
                replayContinuous = false;
                drawReplayStatusText("");
                updateReplayButtonPulse(false);
            }
        }
        break;
    }

    case SCR_IR_SAVED_LIST: {
        if (newScr) {
            drawStatusBar("Saved IR", DV_PINK);
            if (irFileCount == 0) {
                tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
                drawCenteredText("No saved IR signals", 140, DV_CYAN_DIM, 1);
                drawCenteredText("Capture one first", 156, DV_CYAN_DIM, 1);
            } else {
                drawIRSavedList(irFilePtrs, irFileCount, nav.sel(), nav.offset());
            }
            drawFooter("TOP/BOT", "scroll", "HOLD=open");
        }
        if (tevt == TOUCH_NONE || irFileCount == 0) break;
        bool opened = handleMenuTouch(tevt, tx, ty, irFilePtrs, irFileCount, drawMenuWrapper, "Saved IR Signals");
        if (opened) {
            bool loaded = loadIRFromSD(irFileNames[nav.sel()], selectedIR);
            if (loaded) {
                replayPressed    = false;
                replayContinuous = false;
                nav.goTo(SCR_IR_SEND);
            } else {
                drawCenteredText("Load failed!", FOOTER_Y - 16, DV_RED, 1);
            }
        }
        break;
    }

    case SCR_IR_INFO: {
        if (newScr) {
            drawStatusBar("IR Info", DV_PINK);
            drawIRInfoScreen();
            drawFooter("BACK", "return", "");
        }
        break;
    }

    // ══════════════════════════════════════════════════════════
    // SAVED FILES
    // ══════════════════════════════════════════════════════════
    case SCR_FILES_MENU: {
        if (newScr) {
            drawStatusBar("Saved Files", DV_CYAN);
            drawMenu(FILES_ITEMS, FILES_COUNT, nav.sel(), nav.offset(), "SAVED FILES");
            drawFooter("TAP", "select", "HOLD=open");
        }
        if (tevt == TOUCH_NONE) break;
        bool opened = handleMenuTouch(tevt, tx, ty, FILES_ITEMS, FILES_COUNT, drawMenuWrapper, "SAVED FILES");
        if (opened) {
            if (nav.sel() == 0) {
                irFileCount = listIRFiles(irFileNames, IR_FILES_MAX);
                for (uint8_t i = 0; i < irFileCount; i++) irFilePtrs[i] = irFileNames[i];
                nav.goTo(SCR_FILES_IR_LIST);
            } else {
                nfcFileCount = listNFCFiles(nfcFileNames, NFC_FILES_MAX);
                for (uint8_t i = 0; i < nfcFileCount; i++) nfcFilePtrs[i] = nfcFileNames[i];
                nav.goTo(SCR_FILES_NFC_LIST);
            }
        }
        break;
    }

    case SCR_FILES_IR_LIST: {
        if (newScr) {
            drawStatusBar("IR Files", DV_PINK);
            if (irFileCount == 0) {
                tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
                drawCenteredText("No IR files saved", 140, DV_CYAN_DIM, 1);
                drawCenteredText("Capture a signal first", 156, DV_CYAN_DIM, 1);
            } else {
                drawIRSavedList(irFilePtrs, irFileCount, nav.sel(), nav.offset());
            }
            drawFooter("HOLD", "open", "BACK");
        }
        if (tevt == TOUCH_NONE || irFileCount == 0) break;
        bool opened = handleMenuTouch(tevt, tx, ty, irFilePtrs, irFileCount, drawMenuWrapper, "IR Files");
        if (opened) {
            bool loaded = loadIRFromSD(irFileNames[nav.sel()], selectedIR);
            if (loaded) {
                replayPressed    = false;
                replayContinuous = false;
                nav.goTo(SCR_FILES_IR_SEND);
            } else {
                drawCenteredText("Load failed!", FOOTER_Y - 16, DV_RED, 1);
            }
        }
        break;
    }

    case SCR_FILES_IR_SEND: {
        static bool lastPulseIR = false;

        if (newScr) {
            replayPressed    = false;
            replayContinuous = false;
            lastPulseIR      = false;
            drawStatusBar("Transmit IR", DV_PINK);
            drawIRReplayScreen();
            drawReplayStatusText("");
            drawFooter("TAP", "transmit", "BACK");
        }

        if (selectedIR.hasSignal && pulseState != lastPulseIR) {
            lastPulseIR = pulseState;
            updateReplayButtonPulse(pulseState);
        }

        if (!touched) {
            if (replayPressed || replayContinuous) {
                replayPressed    = false;
                replayContinuous = false;
                drawReplayStatusText("");
                updateReplayButtonPulse(false);
            }
            break;
        }

        if (selectedIR.hasSignal && replayButtonHit(tx, ty)) {
            unsigned long now = millis();
            if (!replayPressed) {
                replayPressed    = true;
                replayContinuous = false;
                replayPressStart = now;
                lastReplaySend   = now;
                updateReplayButtonPulse(true);
                bool ok = irSendSignal(selectedIR);
                drawReplayStatusText(ok ? "Sent!" : "Failed!");
                touchClearState();
            } else {
                if (!replayContinuous && (now - replayPressStart >= REPLAY_LONGPRESS_MS)) {
                    replayContinuous = true;
                    lastReplaySend   = now;
                    drawReplayStatusText("Continuous...");
                }
                if (replayContinuous && (now - lastReplaySend >= REPLAY_REPEAT_MS)) {
                    lastReplaySend = now;
                    irSendSignal(selectedIR);
                }
            }
        } else {
            if (replayPressed || replayContinuous) {
                replayPressed    = false;
                replayContinuous = false;
                drawReplayStatusText("");
                updateReplayButtonPulse(false);
            }
        }
        break;
    }

    case SCR_FILES_NFC_LIST: {
        if (newScr) {
            drawStatusBar("NFC Files", DV_PURPLE);
            if (nfcFileCount == 0) {
                tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
                drawCenteredText("No NFC files saved", 140, DV_CYAN_DIM, 1);
                drawCenteredText("Scan a card first", 156, DV_CYAN_DIM, 1);
            } else {
                drawNFCSavedList(nfcFilePtrs, nfcFileCount, nav.sel(), nav.offset());
            }
            drawFooter("HOLD", "emulate", "BACK");
        }
        if (tevt == TOUCH_NONE || nfcFileCount == 0) break;
        bool opened = handleMenuTouch(tevt, tx, ty, nfcFilePtrs, nfcFileCount, drawMenuWrapper, "NFC Files");
        if (opened) {
            bool loaded = loadNFCFromSD(nfcFileNames[nav.sel()], selectedNFC);
            if (loaded) {
                nfcPulse = false; lastNFCPulse = 0;
                nav.goTo(SCR_FILES_NFC_EMULATE);
            } else {
                drawCenteredText("Load failed!", FOOTER_Y - 16, DV_RED, 1);
            }
        }
        break;
    }

    case SCR_FILES_NFC_EMULATE: {
        if (newScr) {
            nfcPulse     = false;
            lastNFCPulse = 0;
            drawStatusBar("NFC Emulate", DV_PURPLE);
            drawNFCEmulate(selectedNFC.uidStr, nfcPulse);
            drawCenteredText(selectedNFC.name, 34, DV_CYAN, 2);
            drawFooter("EMULATE", "active", "BACK");

            if (!nfcStartEmulate(selectedNFC)) {
                drawCenteredText("4-byte UID only!", 200, DV_RED, 1);
                delay(1200);
                nav.goBack();
                break;
            }
        }

        if (millis() - lastNFCPulse > 350) {
            lastNFCPulse = millis();
            nfcPulse     = !nfcPulse;
            updateNFCEmulatePulse(nfcPulse);
        }

        if (nfcEmulateTask()) {
            updateNFCEmulatePulse(true);
            delay(80);
            updateNFCEmulatePulse(nfcPulse);
        }
        break;
    }

    // ══════════════════════════════════════════════════════════
    // SETTINGS
    // ══════════════════════════════════════════════════════════
    case SCR_SETTINGS_MENU: {
        if (newScr) {
            drawStatusBar("Settings", DV_CYAN_DIM);
            drawMenu(SETTINGS_ITEMS, SETTINGS_COUNT, nav.sel(), nav.offset(), "SETTINGS");
            drawFooter("TAP", "open", "BACK");
        }
        if (tevt == TOUCH_NONE) break;
        bool opened = handleMenuTouch(tevt, tx, ty, SETTINGS_ITEMS, SETTINGS_COUNT, drawMenuWrapper, "SETTINGS");
        if (opened) {
            if (nav.sel() == 0) nav.goTo(SCR_SETTINGS_BRIGHTNESS);
            else                nav.goTo(SCR_SETTINGS_ABOUT);
        }
        break;
    }

    case SCR_SETTINGS_BRIGHTNESS: {
        if (newScr) {
            drawStatusBar("Brightness", DV_CYAN_DIM);
            drawBrightness(brightness);
            drawFooter("DRAG", "adjust", "BACK");
        }

        if (!touched) break;

        if (ty >= 128 && ty <= 144 && tx >= 20 && tx <= 220) {
            int newBrightness = constrain(map(tx, 20, 220, 0, 255), 10, 255);
            if (newBrightness != brightness) {
            brightness = newBrightness;
            ledcWrite(TFT_BL_PIN, (uint8_t)brightness);  // ESP32 Core 3.x — pin-based, no channel
            drawBrightnessBarOnly(brightness);
            }
        }
        break;
    }

    case SCR_SETTINGS_ABOUT: {
        if (newScr) {
            drawStatusBar("IRUTESAM v2.0", DV_CYAN);
            drawAbout();
            drawFooter("BACK", "return", "");
        }
        break;
    }

    // ── Boot screen ──────────────────────────────────────────
    case SCR_BOOT:
    default:
        nav.goTo(SCR_HOME);
        break;

    } // end switch
}
