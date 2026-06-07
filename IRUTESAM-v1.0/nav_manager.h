// ════════════════════════════════════════════════════════════
// nav_manager.h
// Universal navigation stack with screen history, state
// preservation, and clean goBack() support.
// ════════════════════════════════════════════════════════════
#ifndef NAV_MANAGER_H
#define NAV_MANAGER_H

#include <Arduino.h>

// ── Screen enum ──────────────────────────────────────────────
enum Screen {
    SCR_BOOT = 0,

    // Top level
    SCR_HOME,

    // ── WiFi ─────────────────────────────────────────────────
    SCR_WIFI_MENU,
    SCR_WIFI_SCAN,
    SCR_WIFI_DETAIL,
    SCR_WIFI_SECURITY,
    SCR_WIFI_INFO,

    // ── NFC ──────────────────────────────────────────────────
    SCR_NFC_MENU,
    SCR_NFC_SCAN,
    SCR_NFC_SAVE_DIALOG,
    SCR_NFC_NAME_INPUT,
    SCR_NFC_EMULATE,
    SCR_NFC_SAVED_LIST,
    SCR_NFC_INFO,

    // ── IR ───────────────────────────────────────────────────
    SCR_IR_MENU,
    SCR_IR_CAPTURE,
    SCR_IR_SAVE_DIALOG,
    SCR_IR_NAME_INPUT,
    SCR_IR_SEND,
    SCR_IR_SAVED_LIST,
    SCR_IR_INFO,

    // ── Saved Files ──────────────────────────────────────────
    SCR_FILES_MENU,
    SCR_FILES_IR_LIST,
    SCR_FILES_IR_SEND,
    SCR_FILES_NFC_LIST,
    SCR_FILES_NFC_EMULATE,

    // ── Settings ─────────────────────────────────────────────
    SCR_SETTINGS_MENU,
    SCR_SETTINGS_BRIGHTNESS,
    SCR_SETTINGS_ABOUT,

    SCR_COUNT
};

// ── Per-screen preserved state ───────────────────────────────
struct ScreenState {
    uint8_t sel;
    uint8_t offset;
};

// ── Navigation manager ───────────────────────────────────────
#define NAV_STACK_DEPTH 12

class NavManager {
public:
    NavManager();

    // Push a new screen. Preserves current sel/offset on stack.
    void goTo(Screen scr, uint8_t initSel = 0, uint8_t initOffset = 0);

    // Pop the stack and return to the previous screen.
    // Returns true if a pop occurred, false if already at root.
    bool goBack();

    // Replace top of stack (no pop, no push — re-enter same screen).
    void replace(Screen scr);

    // Current screen
    Screen current() const { return _stack[_top].scr; }

    // Current selection / offset for this screen
    uint8_t sel()    const { return _stack[_top].sel;    }
    uint8_t offset() const { return _stack[_top].offset; }

    // Update sel/offset for the current screen (call every frame)
    void setSel(uint8_t s)    { _stack[_top].sel    = s; }
    void setOffset(uint8_t o) { _stack[_top].offset = o; }

    // True on the first loop() after a screen change
    bool isNewScreen() const { return _newScreen; }
    void clearNew()          { _newScreen = false; }

    // Stack depth (1 = at root)
    uint8_t depth() const { return _top + 1; }

private:
    struct StackFrame {
        Screen  scr;
        uint8_t sel;
        uint8_t offset;
    };

    StackFrame _stack[NAV_STACK_DEPTH];
    uint8_t    _top;
    bool       _newScreen;
};

extern NavManager nav;

#endif
