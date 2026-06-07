// ════════════════════════════════════════════════════════════
// nfc_module.cpp
// Uses the elechouse PN532 library (NOT Adafruit):
//   Install via Arduino Library Manager → search "PN532"
//   by "Seeed Studio" / elechouse — includes PN532_HSU,
//   EmulateTag, and NdefMessage.
//   Library: https://github.com/elechouse/PN532
// ════════════════════════════════════════════════════════════

#include "nfc_module.h"
#include "config.h"

#include <PN532_HSU.h>
#include <PN532.h>
#include <emulatetag.h>
#include <NdefMessage.h>

// ── UART bus on ESP32 Serial2 ─────────────────────────────────
// config.h: PN532_RX_PIN = 33 (ESP32 RX ← PN532 TX)
//           PN532_TX_PIN = 32 (ESP32 TX → PN532 RX)
// This matches the proven working demo exactly.
static HardwareSerial nfcSerial(2);
static PN532_HSU      pn532hsu(nfcSerial);
static PN532          nfc(pn532hsu);
static EmulateTag     nfc_emulator(pn532hsu);

// ── Public card buffers ───────────────────────────────────────
NFCCard capturedNFC;
NFCCard selectedNFC;

// ── Emulation state ───────────────────────────────────────────
static bool    emulating = false;
static NFCCard emuCard;

// NDEF buffer — 120 bytes is enough for a 4-byte UID MIME record
static uint8_t    ndefBuf[120];
static NdefMessage ndefMsg;

// ════════════════════════════════════════════════════════════
// HELPERS
// ════════════════════════════════════════════════════════════

void nfcClearCard(NFCCard &card) {
    memset(&card, 0, sizeof(NFCCard));
    card.uidLen    = 0;
    card.uidStr[0] = '\0';
    card.name[0]   = '\0';
    card.hasCard   = false;
}

String nfcUidToString(const uint8_t *uid, uint8_t uidLen) {
    String s = "";
    for (uint8_t i = 0; i < uidLen; i++) {
        if (uid[i] < 0x10) s += "0";
        s += String(uid[i], HEX);
        if (i < uidLen - 1) s += " ";
    }
    s.toUpperCase();
    return s;
}

// ════════════════════════════════════════════════════════════
// INIT
// ════════════════════════════════════════════════════════════

void nfcModuleInit() {
    nfcClearCard(capturedNFC);
    nfcClearCard(selectedNFC);
    nfcClearCard(emuCard);
    emulating = false;

    // Start UART — rx pin first, then tx (matches HardwareSerial.begin convention)
    nfcSerial.begin(115200, SERIAL_8N1, PN532_RX_PIN, PN532_TX_PIN);

    // Wake-up byte — required by elechouse library before begin()
    delay(100);
    nfcSerial.write(0x55);
    delay(100);

    nfc.begin();

    uint32_t version = nfc.getFirmwareVersion();
    if (!version) {
        Serial.println("[NFC] PN532 not detected");
        nfcAvailable = false;
        return;
    }

    nfc.SAMConfig();
    nfcAvailable = true;

    Serial.println("[NFC] PN532 ready");
    Serial.print("[NFC] Firmware: 0x");
    Serial.println(version, HEX);
}

// ════════════════════════════════════════════════════════════
// CARD READ (polling — non-blocking with 50 ms timeout)
// ════════════════════════════════════════════════════════════

bool nfcPollCard(NFCCard &card) {
    if (!nfcAvailable) return false;

    uint8_t uid[7];
    uint8_t uidLen = 0;

    // elechouse readPassiveTargetID returns 1 on success, 0 on fail/timeout
    uint8_t ok = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 50);
    if (!ok) return false;

    nfcClearCard(card);

    if (uidLen > NFC_UID_MAX) uidLen = NFC_UID_MAX;
    memcpy(card.uid, uid, uidLen);
    card.uidLen = uidLen;

    String uidText = nfcUidToString(uid, uidLen);
    strncpy(card.uidStr, uidText.c_str(), sizeof(card.uidStr) - 1);
    card.uidStr[sizeof(card.uidStr) - 1] = '\0';

    strncpy(card.name, "CARD", sizeof(card.name) - 1);
    card.name[sizeof(card.name) - 1] = '\0';

    card.hasCard = true;
    capturedNFC  = card;

    Serial.println("========== NFC READ ==========");
    Serial.print("UID LEN : "); Serial.println(card.uidLen);
    Serial.print("UID     : "); Serial.println(card.uidStr);
    Serial.println("==============================");

    delay(250); // debounce — prevent double-read
    return true;
}

// ════════════════════════════════════════════════════════════
// CARD EMULATION
// Mirrors the proven working demo exactly:
//   nfc_emulator.setNdefFile()  — load NDEF payload
//   nfc_emulator.setUid()       — set the UID to present
//   nfc_emulator.init()         — switch PN532 to target mode
//   nfc_emulator.emulate()      — service one reader exchange (non-blocking)
// ════════════════════════════════════════════════════════════

bool nfcStartEmulate(const NFCCard &card) {
    if (!nfcAvailable)                     return false;
    if (!card.hasCard || card.uidLen == 0) return false;

    // Emulation only works reliably with a 4-byte UID (MIFARE Classic).
    // 7-byte UIDs (MIFARE Ultralight) are not supported by EmulateTag.
    if (card.uidLen != 4) {
        Serial.println("[NFC] Emulation requires a 4-byte UID");
        return false;
    }

    memcpy(&emuCard, &card, sizeof(NFCCard));

    // Build an NDEF message carrying the raw UID bytes as a MIME payload.
    // This is identical to the working demo.
    ndefMsg = NdefMessage();
    ndefMsg.addMimeMediaRecord(
        "application/octet-stream",
        emuCard.uid,
        4
    );

    int msgSize = ndefMsg.getEncodedSize();
    ndefMsg.encode(ndefBuf);

    nfc_emulator.setNdefFile(ndefBuf, msgSize);
    nfc_emulator.setUid(emuCard.uid);   // present this UID to readers
    nfc_emulator.init();                // switches PN532 into PICC/target mode

    emulating = true;

    Serial.print("[NFC] Emulation started — UID: ");
    Serial.println(card.uidStr);
    return true;
}

void nfcStopEmulate() {
    if (!emulating) return;
    emulating = false;

    // Return PN532 to normal reader/initiator mode
    nfc.begin();
    nfc.SAMConfig();

    Serial.println("[NFC] Emulation stopped — reader mode restored");
}

// Call every loop() iteration while in the emulate screen.
// emulate(0) is non-blocking: returns immediately if no reader present.
// Returns true when a reader completes a tag-read transaction.
bool nfcEmulateTask() {
    if (!emulating || !nfcAvailable) return false;
    // Pass timeout=0 for non-blocking behaviour so the UI loop
    // keeps running and the touch/back button stays responsive.
    return (nfc_emulator.emulate(0) == 1);
}