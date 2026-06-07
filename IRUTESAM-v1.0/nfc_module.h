#ifndef NFC_MODULE_H
#define NFC_MODULE_H

#include <Arduino.h>

// ── Only 4-byte UIDs are supported for emulation (MIFARE Classic) ──
#define NFC_UID_MAX   7    // read up to 7 bytes; emulate only uses first 4
#define NFC_NAME_MAX  12
#define NFC_FILES_MAX 32

struct NFCCard {
    uint8_t uid[NFC_UID_MAX];
    uint8_t uidLen;
    char    uidStr[3 * NFC_UID_MAX + 1];
    char    name[NFC_NAME_MAX + 1];
    bool    hasCard;
};

// ── Init / Read ──────────────────────────────────────────────
void   nfcModuleInit();
bool   nfcPollCard(NFCCard &card);
void   nfcClearCard(NFCCard &card);
String nfcUidToString(const uint8_t *uid, uint8_t uidLen);

// ── Card Emulation ───────────────────────────────────────────
// Call nfcStartEmulate() when entering the emulate screen.
// Call nfcEmulateTask() every loop() iteration (non-blocking).
// Call nfcStopEmulate() when leaving the screen.
bool nfcStartEmulate(const NFCCard &card);
void nfcStopEmulate();
bool nfcEmulateTask();

extern NFCCard capturedNFC;
extern NFCCard selectedNFC;

#endif