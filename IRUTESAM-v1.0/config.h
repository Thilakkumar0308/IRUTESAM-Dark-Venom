#ifndef CONFIG_H
#define CONFIG_H

// ── SPI SHARED: TFT + TOUCH ─────────────────────────────────
#define PIN_MOSI        23
#define PIN_MISO        19
#define PIN_SCK         18

// ── TFT ILI9341 ─────────────────────────────────────────────
#define TFT_CS_PIN      17
#define TFT_DC_PIN      16
#define TFT_RST_PIN      5

// ── TOUCH XPT2046 ───────────────────────────────────────────
#define TOUCH_CS_PIN    25
#define TOUCH_IRQ_PIN   22

// ── TFT BACKLIGHT ───────────────────────────────────────────
#define TFT_BL_PIN      21

// ── SD CARD : SEPARATE SPI BUS ──────────────────────────────
#define SD_CS_PIN       27
#define SD_MOSI_PIN     12
#define SD_MISO_PIN     13
#define SD_SCK_PIN      14

// ── IR ──────────────────────────────────────────────────────
#define IR_TX_PIN        4
#define IR_RX_PIN       15

// ── IR BUFFER ───────────────────────────────────────────────
#define IR_MAX_RAW      1024
#define IR_CAPTURE_BUF  1024
#define IR_TIMEOUT_MS   50
#define IR_NAME_MAX     12
#define IR_FILES_MAX    32

// ---- NFC PN532 TX and RX -------------
// ── PN532 UART ───────────────────────────────────────────────
#define PN532_RX_PIN     33
#define PN532_TX_PIN     32

#define NFC_UID_MAX      10
#define NFC_NAME_MAX     12
#define NFC_FILES_MAX    32

// ── SCREEN LAYOUT ───────────────────────────────────────────
#define SCREEN_W        240
#define SCREEN_H        320
#define STATUS_H         16
#define FOOTER_H         22
#define FOOTER_Y        298

#define MENU_START_Y     34
#define MENU_ROW_H       44
#define MENU_MAX_ROWS     5

#define WIFI_LIST_START_Y 34
#define WIFI_LIST_ROW_H   30
#define WIFI_LIST_MAX_VIS  8

// ════════════════════════════════════════════════════════════
// DARK BLUE THEME — RGB565 values pre-swapped for TFT_BGR
//
// RGB565 bit layout: RRRRRGGGGGGBBBBB
// For BGR panels:    BBBBBGGGGGGRRRRR
//
// To get a color on BGR screen, swap R and B in your RGB value.
// Formula: take RGB hex → swap R↔B → encode as RGB565
//
// Target palette:
//   Background      : #000820  very dark navy
//   Panel/header    : #001040  dim navy
//   Sky blue text   : #4DB8FF  bright sky blue
//   Mid blue        : #1E6FBF  medium blue (highlight bg)
//   Dim blue        : #0A3060  muted blue
//   Accent purple   : #7B2FBE
//   Accent pink     : #FF2D6B
//   Green           : #00E676
//   Red             : #FF1744
//   Amber           : #FFAB00
// ════════════════════════════════════════════════════════════

// Helper macro: build RGB565 from raw R,G,B values
// For BGR panel: we encode with R and B swapped so the
// hardware swap brings them back to what we intended.
// i.e. we pass (B,G,R) into the standard RGB565 formula.
#define RGB565_BGR(r,g,b) ( (uint16_t)(((b) & 0xF8) << 8) | \
                            (uint16_t)(((g) & 0xFC) << 3) | \
                            (uint16_t)(((r) & 0xF8) >> 3) )

// ── Background colors ────────────────────────────────────────
// #000820 → R=0x00 G=0x08 B=0x20
#define DV_BG       RGB565_BGR(0x00, 0x10, 0x30)  // very dark navy
// #001040 → R=0x00 G=0x10 B=0x40
#define DV_BG2      RGB565_BGR(0x00, 0x10, 0x40)   // dim navy (header/footer/panel)
#define DV_PANEL    RGB565_BGR(0x00, 0x10, 0x40)   // same as BG2

// ── Primary text — sky blue ──────────────────────────────────
// #4DB8FF → R=0x4D G=0xB8 B=0xFF
#define DV_CYAN     RGB565_BGR(0x4D, 0xB8, 0xFF)   // sky blue — main text

// ── Mid blue — border / subtle elements ─────────────────────
// #1E6FBF → R=0x1E G=0x6F B=0xBF
#define DV_CYAN_MID RGB565_BGR(0x1E, 0x6F, 0xBF)   // medium blue

// ── Dim blue — secondary/disabled text ──────────────────────
// #0A3060 → R=0x0A G=0x30 B=0x60
#define DV_CYAN_DIM RGB565_BGR(0x0A, 0x30, 0x60)   // dim blue

// ── Selection highlight background ──────────────────────────
// #0D4A8C → R=0x0D G=0x4A B=0x8C  (mid-dark blue, not too bright)
#define DV_HILIGHT  RGB565_BGR(0x0D, 0x4A, 0x8C)   // highlight bg

// ── Accent colors ────────────────────────────────────────────
// #7B2FBE → R=0x7B G=0x2F B=0xBE
#define DV_PURPLE   RGB565_BGR(0x7B, 0x2F, 0xBE)

// #FF2D6B → R=0xFF G=0x2D B=0x6B
#define DV_PINK     RGB565_BGR(0xFF, 0x2D, 0x6B)

// ── Status colors ────────────────────────────────────────────
#define DV_GREEN    RGB565_BGR(0x00, 0xE6, 0x76)
#define DV_RED      RGB565_BGR(0xFF, 0x17, 0x44)
#define DV_AMBER    RGB565_BGR(0xFF, 0xAB, 0x00)
#define DV_WHITE    RGB565_BGR(0xFF, 0xFF, 0xFF)

// ── Filled variants for badges ───────────────────────────────
#define DV_RED_FILL RGB565_BGR(0x3F, 0x00, 0x10)
#define DV_AMB_FILL RGB565_BGR(0x3F, 0x2A, 0x00)
#define DV_GRN_FILL RGB565_BGR(0x00, 0x3F, 0x1A)

// ── Security type constants ──────────────────────────────────
#define SEC_OPEN    0
#define SEC_WEP     1
#define SEC_WPA     2
#define SEC_WPA2    3
#define SEC_WPA3    4

// ── Shared global flags ──────────────────────────────────────
extern bool wifiReady;
extern bool sdAvailable;
extern bool nfcAvailable;
extern bool touchAvailable;
extern int  directRowIndex;

enum InputAction {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_SELECT,
    INPUT_BACK,
    INPUT_DIRECT_ROW,
    INPUT_TOUCH
};

#endif