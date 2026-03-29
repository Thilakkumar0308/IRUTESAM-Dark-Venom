// ════════════════════════════════════════
// IRUTESAM — Dark Venom  |  v1.1
// FILE: IRUTESAM.ino
// BOARD: ESP32 DevKit V1
// DISPLAY: ILI9341 240x320 PORTRAIT
// ════════════════════════════════════════

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "config.h"
#include "touch_input.h"
#include "ir_module.h"
#include "sd_manager.h"
#include "tft_ui.h"

// ── Global flags ─────────────────────────────────────────────
bool wifiReady      = false;
bool sdAvailable    = false;
bool nfcAvailable   = false;
bool touchAvailable = false;
int  directRowIndex = -1;

extern TFT_eSPI tft;

// ════════════════════════════════════════
// UI STATE MACHINE
// ════════════════════════════════════════
enum UIState {
    UI_BOOT,
    UI_HOME,
    UI_IR_CAPTURE,
    UI_IR_SAVE_DIALOG,
    UI_IR_NAME_INPUT,
    UI_IR_SAVED_LIST,
    UI_IR_REPLAY,
    UI_SETTINGS,
    UI_SETTINGS_BRIGHTNESS,
    UI_SETTINGS_ABOUT
};

static UIState currentState = UI_BOOT;
static UIState prevState    = UI_BOOT;
static bool    stateChanged = true;

// ── Home menu ────────────────────────────────────────────────
static const char *HOME_ITEMS[] = {
    "IR Receiver",
    "IR Replay Signal",
    "Saved Files",
    "Settings"
};
static const uint8_t HOME_COUNT = 4;
static uint8_t homesel = 0, homeoffset = 0;

// ── Settings menu ────────────────────────────────────────────
static const char *SETTINGS_ITEMS[] = {
    "Brightness",
    "About"
};
static const uint8_t SETTINGS_COUNT = 2;
static uint8_t setsel = 0, setoffset = 0;

// ── Saved files list ─────────────────────────────────────────
static char        irFileNames[IR_FILES_MAX][IR_NAME_MAX + 5];
static const char *irFilePointers[IR_FILES_MAX];
static uint8_t     irFileCount = 0;
static uint8_t     filesel = 0, fileoffset = 0;

// ── Save dialog ──────────────────────────────────────────────
static bool dialogYes = true;

// ── Name input keyboard ──────────────────────────────────────
static char    inputName[IR_NAME_MAX + 1];
static uint8_t inputLen = 0;

// ── Brightness ───────────────────────────────────────────────
static int brightness = 180;

// ── Animation / pulse ────────────────────────────────────────
static unsigned long lastPulse  = 0;
static bool          pulseState = false;

// ── Replay press / hold control ──────────────────────────────
static bool          replayPressed    = false;
static bool          replayContinuous = false;
static unsigned long replayPressStart = 0;
static unsigned long lastReplaySend   = 0;

#define REPLAY_LONGPRESS_MS  500
#define REPLAY_REPEAT_MS     180

// ════════════════════════════════════════
// KEYBOARD LAYOUT
// ════════════════════════════════════════
static const char KB_KEYS[] =
    "ABCDEFGHIJ"
    "KLMNOPQRST"
    "UVWXYZ0123"
    "456789";

#define KB_COLS     10
#define KB_KEY_W    21
#define KB_KEY_H    24
#define KB_START_X   4
#define KB_START_Y  78
#define KB_GAP       2

// ════════════════════════════════════════
// INTERNAL HELPERS
// ════════════════════════════════════════
static void setState(UIState s) {
    if (s == currentState) return;
    prevState    = currentState;
    currentState = s;
    stateChanged = true;
    directRowIndex = -1;
    touchClearState();
}

static void drawReplayStatusText(const char *msg) {
    tft.fillRect(0, 248, SCREEN_W, 16, DV_BG);
    drawCenteredText(msg, 252, DV_CYAN_DIM, 1);
}

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

static bool replayButtonHit(int tx, int ty) {
    return (tx >= 25 && tx <= 215 && ty >= 140 && ty <= 184);
}

static bool replayBackHit(int tx, int ty) {
    return (tx >= 70 && tx <= 170 && ty >= 208 && ty <= 230);
}

// ════════════════════════════════════════
// KEYBOARD SCREEN
// ════════════════════════════════════════
static void drawKeyboard() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);

    tft.setTextSize(1);
    tft.setTextColor(DV_PURPLE, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// Enter Signal Name");

    tft.fillRoundRect(8, 36, 224, 22, 3, DV_PANEL);
    tft.drawRoundRect(8, 36, 224, 22, 3, DV_CYAN_MID);

    char displayBuf[IR_NAME_MAX + 2];
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
    int saveX = 64;
    int saveW = 172;

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
            if (inputLen < IR_NAME_MAX) {
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
    int saveX = 64;
    int saveW = 172;

    if (tx >= 4 && tx <= 52 && ty >= delY && ty <= delY + 24) {
        if (inputLen > 0) {
            inputLen--;
            inputName[inputLen] = '\0';
        }
        drawKeyboard();
        return;
    }

    if (tx >= saveX && tx <= saveX + saveW &&
        ty >= delY  && ty <= delY + 24) {
        if (inputLen > 0) {
            strncpy(capturedIR.name, inputName, IR_NAME_MAX);
            capturedIR.name[IR_NAME_MAX] = '\0';

            bool saved = saveIRToSD(capturedIR);

            tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
            tft.setTextSize(2);
            drawCenteredText(saved ? "SAVED!" : "SAVE FAILED", 140, DV_CYAN, 2);
            delay(600);
            setState(UI_HOME);
        }
    }
}

// ════════════════════════════════════════
// IR REPLAY SCREEN
// ════════════════════════════════════════
static void drawIRReplayScreen() {
    tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);

    tft.setTextSize(1);
    tft.setTextColor(DV_PINK, DV_BG);
    tft.setCursor(6, 20);
    tft.print("// IR Replay");

    if (!selectedIR.hasSignal) {
        drawCenteredText("No signal selected",      96, DV_CYAN,     1);
        drawCenteredText("Go to Saved Files first", 112, DV_CYAN_DIM, 1);
        return;
    }

    drawCenteredText(selectedIR.name, 40, DV_CYAN, 2);

    char infoBuf[32];
    snprintf(infoBuf, sizeof(infoBuf), "%s  %u-bit",
             typeToString(selectedIR.protocol).c_str(),
             selectedIR.bits);
    drawCenteredText(infoBuf, 62, DV_CYAN_DIM, 1);

    tft.fillRoundRect(8, 80, 224, 28, 3, DV_PANEL);
    tft.drawRoundRect(8, 80, 224, 28, 3, DV_CYAN_MID);

    if (selectedIR.rawLen > 0) {
        uint32_t maxV = 1;
        for (uint16_t i = 0; i < selectedIR.rawLen; i++)
            if (selectedIR.rawData[i] > maxV) maxV = selectedIR.rawData[i];

        int xPos     = 10;
        int drawW    = 220;
        int pxPerSample = max(1, drawW / (int)selectedIR.rawLen);

        for (uint16_t i = 0; i < selectedIR.rawLen && xPos < 230; i++) {
            int w = max(1, (int)((selectedIR.rawData[i] * pxPerSample * 4) / maxV));
            if (xPos + w > 230) w = 230 - xPos;
            uint16_t col = (i % 2 == 0) ? DV_CYAN : DV_CYAN_MID;
            int barY = (i % 2 == 0) ? 84 : 96;
            tft.fillRect(xPos, barY, w, 10, col);
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

    int btnW = 100, btnH = 22, btnX = 70, btnY = 208;
    tft.fillRoundRect(btnX, btnY, btnW, btnH, 3, DV_BG2);
    tft.drawRoundRect(btnX, btnY, btnW, btnH, 3, DV_CYAN_MID);
    tft.setTextSize(1);
    tft.setTextColor(DV_CYAN, DV_BG2);
    tft.setCursor(btnX + 34, btnY + 7);
    tft.print("BACK");
}

static void updateReplayButtonPulse(bool glow) {
    int bx = 25, by = 140, bw = 190, bh = 44;
    tft.fillRoundRect(bx, by, bw, bh, 4, glow ? DV_HILIGHT : DV_BG2);
    tft.drawRoundRect(bx, by, bw, bh, 4, glow ? DV_CYAN    : DV_CYAN_MID);
    tft.setTextSize(2);
    tft.setTextColor(glow ? DV_CYAN : DV_CYAN_DIM,
                     glow ? DV_HILIGHT : DV_BG2);
    tft.setCursor(bx + 42, by + 12);
    tft.print("REPLAY");
}

// ════════════════════════════════════════
// SAVE DIALOG
// ════════════════════════════════════════
static void drawSaveDialog() {
    drawIRCapture(capturedIR.rawData, capturedIR.rawLen);
    drawStatusBar("IR Receiver", DV_PINK);
    drawDialog("Save Signal?",
               "Store to SD card?",
               capturedIR.hasSignal ? "Signal captured" : "",
               dialogYes);
}

// ════════════════════════════════════════
// SETUP
// ════════════════════════════════════════
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

    setState(UI_HOME);
    stateChanged = true;
}

// ════════════════════════════════════════
// LOOP
// ════════════════════════════════════════
void loop() {
    TouchEvent tevt  = touchPoll();
    bool       touched = (tevt != TOUCH_NONE);
    int        tx    = getLastTouchX();
    int        ty    = getLastTouchY();

    if (millis() - lastPulse > 380) {
        lastPulse  = millis();
        pulseState = !pulseState;
    }

    switch (currentState) {

    // ══════════════════════════════════════
    case UI_HOME: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("IRUTESAM", DV_CYAN);
            drawMenu(HOME_ITEMS, HOME_COUNT, homesel, homeoffset, "MAIN MENU");
            drawFooter("TAP", "select", "");
        }

        if (!touched) break;

        for (uint8_t r = 0; r < MENU_MAX_ROWS; r++) {
            uint8_t idx = homeoffset + r;
            if (idx >= HOME_COUNT) break;

            int rowY = MENU_START_Y + r * MENU_ROW_H;
            if (ty < rowY || ty >= rowY + MENU_ROW_H) continue;

            if (homesel != idx) {
                if (homesel >= homeoffset &&
                    homesel < homeoffset + MENU_MAX_ROWS) {
                    int oldRowY = MENU_START_Y + (homesel - homeoffset) * MENU_ROW_H;
                    drawMenuRowPartial(HOME_ITEMS[homesel], oldRowY, false);
                }
                homesel = idx;
                drawMenuRowPartial(HOME_ITEMS[homesel], rowY, true);
            }

            switch (homesel) {
                case 0:
                    irStartCapture();
                    irClearSignal(capturedIR);
                    setState(UI_IR_CAPTURE);
                    break;
                case 1:
                    setState(UI_IR_REPLAY);
                    break;
                case 2:
                    irFileCount = listIRFiles(irFileNames, IR_FILES_MAX);
                    for (uint8_t i = 0; i < irFileCount; i++)
                        irFilePointers[i] = irFileNames[i];
                    filesel    = 0;
                    fileoffset = 0;
                    setState(UI_IR_SAVED_LIST);
                    break;
                case 3:
                    setsel    = 0;
                    setoffset = 0;
                    setState(UI_SETTINGS);
                    break;
            }
            break;
        }
        break;
    }

    // ══════════════════════════════════════
    case UI_IR_CAPTURE: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("IR Receiver", DV_PINK);
            drawIRCapture(capturedIR.rawData, capturedIR.rawLen);
            drawFooter("Point", "remote", "BACK");
        }

        if (irPollCapture(capturedIR)) {
            drawIRCapture(capturedIR.rawData, capturedIR.rawLen);

            char infoBuf[28];
            snprintf(infoBuf, sizeof(infoBuf), "%s  0x%lX",
                     typeToString(capturedIR.protocol).c_str(),
                     (uint32_t)capturedIR.value);
            drawCenteredText(infoBuf, 116, DV_CYAN, 1);

            dialogYes = true;
            setState(UI_IR_SAVE_DIALOG);
        }

        if (touched && ty >= FOOTER_Y) {
            irStopCapture();
            touchClearState();
            setState(UI_HOME);
        }
        break;
    }

    // ══════════════════════════════════════
    case UI_IR_SAVE_DIALOG: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("IR Receiver", DV_PINK);
            drawSaveDialog();
            drawFooter("YES", "save", "NO");
        }

        if (touched) {
            bool changed = checkDialogTouch(dialogYes);
            if (changed) {
                drawSaveDialog();
                if (dialogYes) {
                    memset(inputName, 0, sizeof(inputName));
                    inputLen = 0;
                    setState(UI_IR_NAME_INPUT);
                } else {
                    irStartCapture();
                    setState(UI_IR_CAPTURE);
                }
            }
        }
        break;
    }

    // ══════════════════════════════════════
    case UI_IR_NAME_INPUT: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("Save Signal", DV_GREEN);
            drawKeyboard();
            drawFooter("TYPE", "name", "SAVE");
        }

        if (touched) {
            handleKeyboardTouch(tx, ty);
        }
        break;
    }

    // ══════════════════════════════════════
    case UI_IR_SAVED_LIST: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("Saved Files", DV_CYAN);
            if (irFileCount == 0) {
                tft.fillRect(0, STATUS_H, SCREEN_W, FOOTER_Y - STATUS_H, DV_BG);
                drawCenteredText("No saved signals",   140, DV_CYAN_DIM, 1);
                drawCenteredText("Capture one first",  156, DV_CYAN_DIM, 1);
            } else {
                drawIRSavedList(irFilePointers, irFileCount, filesel, fileoffset);
            }
            drawFooter("TOP/BOT", "scroll", "HOLD=open");
        }

        if (tevt == TOUCH_NONE) break;

        // ── Footer / status bar tap → BACK ───────────────────
        if (tevt == TOUCH_RAW) {
            if (ty >= FOOTER_Y || ty < STATUS_H) {
                touchClearState();
                setState(UI_HOME);
            }
            break;
        }

        // ── Top zone → scroll up ─────────────────────────────
        if (tevt == TOUCH_UP) {
            if (irFileCount == 0) break;
            if (filesel > 0) {
                uint8_t oldSel = filesel;
                filesel--;
                if (filesel < fileoffset) {
                    fileoffset = filesel;
                    drawIRSavedList(irFilePointers, irFileCount, filesel, fileoffset);
                } else {
                    int oldRow = MENU_START_Y + (oldSel  - fileoffset) * MENU_ROW_H;
                    int newRow = MENU_START_Y + (filesel - fileoffset) * MENU_ROW_H;
                    drawMenuRowPartial(irFilePointers[oldSel],  oldRow, false);
                    drawMenuRowPartial(irFilePointers[filesel], newRow, true);
                }
            }
            break;
        }

        // ── Bottom zone → scroll down ────────────────────────
        if (tevt == TOUCH_DOWN) {
            if (irFileCount == 0) break;
            if (filesel < irFileCount - 1) {
                uint8_t oldSel = filesel;
                filesel++;
                if (filesel >= fileoffset + MENU_MAX_ROWS) {
                    fileoffset = filesel - MENU_MAX_ROWS + 1;
                    drawIRSavedList(irFilePointers, irFileCount, filesel, fileoffset);
                } else {
                    int oldRow = MENU_START_Y + (oldSel  - fileoffset) * MENU_ROW_H;
                    int newRow = MENU_START_Y + (filesel - fileoffset) * MENU_ROW_H;
                    drawMenuRowPartial(irFilePointers[oldSel],  oldRow, false);
                    drawMenuRowPartial(irFilePointers[filesel], newRow, true);
                }
            }
            break;
        }

        // ── Centre single tap → flash selection border ───────
        if (tevt == TOUCH_SELECT) {
            if (irFileCount > 0 &&
                filesel >= fileoffset &&
                filesel < fileoffset + MENU_MAX_ROWS) {
                int rowY = MENU_START_Y + (filesel - fileoffset) * MENU_ROW_H;
                tft.drawRect(0, rowY, SCREEN_W - 4, MENU_ROW_H - 2, DV_CYAN);
            }
            break;
        }

        // ── Centre double-tap or hold → load & open ──────────
        if (tevt == TOUCH_OPEN) {
            if (irFileCount == 0) break;
            bool loaded = loadIRFromSD(irFileNames[filesel], selectedIR);
            Serial.print("Loading: ");
            Serial.println(irFileNames[filesel]);
            Serial.println(loaded ? "LOAD OK" : "LOAD FAIL");
            touchClearState();
            if (loaded) {
                setState(UI_IR_REPLAY);
            } else {
                tft.fillRect(0, FOOTER_Y - 18, SCREEN_W, 14, DV_BG);
                drawCenteredText("Load failed!", FOOTER_Y - 16, DV_RED, 1);
            }
            break;
        }

        break;
    }

    // ══════════════════════════════════════
    case UI_IR_REPLAY: {
        if (stateChanged) {
            stateChanged     = false;
            replayPressed    = false;
            replayContinuous = false;
            drawStatusBar("IR Replay", DV_PINK);
            drawIRReplayScreen();
            drawReplayStatusText("");
            drawFooter("TAP", "replay", "BACK");
        }

        static bool lastReplayPulseState = false;
        if (selectedIR.hasSignal && pulseState != lastReplayPulseState) {
            lastReplayPulseState = pulseState;
            updateReplayButtonPulse(pulseState);
        }

        // ── No touch — reset replay state ────────────────────
        if (!touched) {
            if (replayPressed || replayContinuous) {
                replayPressed    = false;
                replayContinuous = false;
                drawReplayStatusText("");
                updateReplayButtonPulse(false);
            }
            break;
        }

        // ── BACK button ──────────────────────────────────────
        if (replayBackHit(tx, ty)) {
            replayPressed    = false;
            replayContinuous = false;
            touchClearState();
            setState(UI_HOME);
            break;
        }

        // ── REPLAY button ────────────────────────────────────
        if (selectedIR.hasSignal && replayButtonHit(tx, ty)) {
            unsigned long now = millis();

            if (!replayPressed) {
                // First press — fire once
                replayPressed    = true;
                replayContinuous = false;
                replayPressStart = now;
                lastReplaySend   = now;

                updateReplayButtonPulse(true);
                bool ok = irSendSignal(selectedIR);
                drawReplayStatusText(ok ? "Sent!" : "Failed!");
                touchClearState();
            } else {
                // Held — promote to continuous after threshold
                if (!replayContinuous &&
                    (millis() - replayPressStart >= REPLAY_LONGPRESS_MS)) {
                    replayContinuous = true;
                    lastReplaySend   = millis();
                    drawReplayStatusText("Continuous...");
                }
                if (replayContinuous &&
                    (millis() - lastReplaySend >= REPLAY_REPEAT_MS)) {
                    lastReplaySend = millis();
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

    // ══════════════════════════════════════
    case UI_SETTINGS: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("Settings", DV_CYAN_DIM);
            drawMenu(SETTINGS_ITEMS, SETTINGS_COUNT, setsel, setoffset, "Settings");
            drawFooter("TAP", "open", "BACK");
        }

        if (!touched) break;

        if (ty >= FOOTER_Y) {
            touchClearState();
            setState(UI_HOME);
            break;
        }

        for (uint8_t r = 0; r < MENU_MAX_ROWS; r++) {
            uint8_t idx = setoffset + r;
            if (idx >= SETTINGS_COUNT) break;

            int rowY = MENU_START_Y + r * MENU_ROW_H;
            if (ty < rowY || ty >= rowY + MENU_ROW_H) continue;

            if (setsel != idx) {
                if (setsel >= setoffset &&
                    setsel < setoffset + MENU_MAX_ROWS) {
                    int oldRowY = MENU_START_Y + (setsel - setoffset) * MENU_ROW_H;
                    drawMenuRowPartial(SETTINGS_ITEMS[setsel], oldRowY, false);
                }
                setsel = idx;
                drawMenuRowPartial(SETTINGS_ITEMS[setsel], rowY, true);
            }

            if (idx == 0) setState(UI_SETTINGS_BRIGHTNESS);
            else          setState(UI_SETTINGS_ABOUT);
            break;
        }
        break;
    }

    // ══════════════════════════════════════
    case UI_SETTINGS_BRIGHTNESS: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("Brightness", DV_CYAN_DIM);
            drawBrightness(brightness);
            drawFooter("DRAG", "adjust", "BACK");
        }

        if (!touched) break;

        if (ty >= FOOTER_Y) {
            touchClearState();
            setState(UI_SETTINGS);
            break;
        }

        if (ty >= 128 && ty <= 144 && tx >= 20 && tx <= 220) {
            int newBrightness = constrain(map(tx, 20, 220, 0, 255), 10, 255);
            if (newBrightness != brightness) {
                brightness = newBrightness;
                drawBrightnessBarOnly(brightness);
            }
        }
        break;
    }

    // ══════════════════════════════════════
    case UI_SETTINGS_ABOUT: {
        if (stateChanged) {
            stateChanged = false;
            drawStatusBar("IRUTESAM v1.1", DV_CYAN);
            drawAbout();
            drawFooter("BCK", "back", "");
        }

        if (touched && ty >= FOOTER_Y) {
            touchClearState();
            setState(UI_SETTINGS);
        }
        break;
    }

    // ══════════════════════════════════════
    default:
        setState(UI_HOME);
        stateChanged = true;
        break;

    } // end switch
}   // end loop
