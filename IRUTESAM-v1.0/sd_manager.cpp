#include "sd_manager.h"
#include <SPI.h>

SPIClass sdSPI(HSPI);

// ─────────────────────────────────────────────
// INIT
// ─────────────────────────────────────────────
bool sdInit() {
    sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    if (!SD.begin(SD_CS_PIN, sdSPI)) {
        Serial.println("SD FAIL");
        return false;
    }

    Serial.println("SD OK");
    return true;
}

// ─────────────────────────────────────────────
// IR SAVE
// ─────────────────────────────────────────────
bool saveIRToSD(const IRSignal &sig) {
    if (!sig.hasSignal) return false;

    char filename[32];
    snprintf(filename, sizeof(filename), "/%s.ir", sig.name);

    File file = SD.open(filename, FILE_WRITE);
    if (!file) return false;

    file.printf("%u,%llu,%u,%u\n",
                sig.protocol, sig.value, sig.bits, sig.rawLen);

    for (uint16_t i = 0; i < sig.rawLen; i++) {
        file.print(sig.rawData[i]);
        if (i < sig.rawLen - 1) file.print(",");
    }

    file.println();
    file.close();
    return true;
}

// ─────────────────────────────────────────────
// IR LOAD
// ─────────────────────────────────────────────
bool loadIRFromSD(const char *filename, IRSignal &sig) {
    char fullpath[40];
    snprintf(fullpath, sizeof(fullpath), "/%s", filename);

    File file = SD.open(fullpath);
    if (!file) {
        Serial.print("[SD] File not found: ");
        Serial.println(fullpath);
        sig.hasSignal = false;
        return false;
    }

    irClearSignal(sig);

    String header = file.readStringUntil('\n');
    header.trim();

    sscanf(header.c_str(), "%u,%llu,%u,%u",
           (unsigned int*)&sig.protocol,
           &sig.value,
           &sig.bits,
           &sig.rawLen);

    if (sig.rawLen > IR_MAX_RAW) sig.rawLen = IR_MAX_RAW;

    String raw = file.readString();
    file.close();

    char buffer[2048];
    raw.toCharArray(buffer, sizeof(buffer));

    int idx = 0;
    char *token = strtok(buffer, ",");
    while (token != NULL && idx < (int)sig.rawLen) {
        sig.rawData[idx++] = (uint16_t)atoi(token);
        token = strtok(NULL, ",");
    }

    sig.hasSignal = true;

    strncpy(sig.name, filename, IR_NAME_MAX);
    sig.name[IR_NAME_MAX] = '\0';
    char *dot = strrchr(sig.name, '.');
    if (dot) *dot = '\0';

    return true;
}

// ─────────────────────────────────────────────
// IR LIST
// ─────────────────────────────────────────────
uint8_t listIRFiles(char files[][IR_NAME_MAX + 5], uint8_t maxFiles) {
    File root = SD.open("/");
    if (!root) return 0;

    uint8_t count = 0;

    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;

        String name = entry.name();
        if (name.endsWith(".ir") && count < maxFiles) {
            strncpy(files[count], name.c_str(), IR_NAME_MAX + 4);
            files[count][IR_NAME_MAX + 4] = '\0';
            count++;
        }

        entry.close();
    }

    root.close();
    return count;
}

// ─────────────────────────────────────────────
// IR DELETE
// ─────────────────────────────────────────────
bool deleteIRFromSD(const char *filename) {
    char fullpath[40];
    snprintf(fullpath, sizeof(fullpath), "/%s", filename);
    return SD.remove(fullpath);
}

// ─────────────────────────────────────────────
// NFC SAVE
// ─────────────────────────────────────────────
bool saveNFCToSD(const NFCCard &card) {
    if (!card.hasCard || card.uidLen == 0) return false;

    char filename[32];
    snprintf(filename, sizeof(filename), "/%s.nfc", card.name);

    File file = SD.open(filename, FILE_WRITE);
    if (!file) return false;

    file.printf("%u\n", card.uidLen);
    file.println(card.uidStr);

    file.close();
    return true;
}

// ─────────────────────────────────────────────
// NFC LOAD
// ─────────────────────────────────────────────
bool loadNFCFromSD(const char *filename, NFCCard &card) {
    char fullpath[40];
    snprintf(fullpath, sizeof(fullpath), "/%s", filename);

    File file = SD.open(fullpath);
    if (!file) {
        Serial.print("[SD] NFC file not found: ");
        Serial.println(fullpath);
        nfcClearCard(card);
        return false;
    }

    nfcClearCard(card);

    String lenLine = file.readStringUntil('\n');
    String uidLine = file.readStringUntil('\n');
    lenLine.trim();
    uidLine.trim();

    card.uidLen = (uint8_t)lenLine.toInt();
    if (card.uidLen > NFC_UID_MAX) card.uidLen = NFC_UID_MAX;

    strncpy(card.uidStr, uidLine.c_str(), sizeof(card.uidStr) - 1);
    card.uidStr[sizeof(card.uidStr) - 1] = '\0';

    char buf[64];
    uidLine.toCharArray(buf, sizeof(buf));

    uint8_t idx = 0;
    char *token = strtok(buf, " ");
    while (token != NULL && idx < card.uidLen) {
        card.uid[idx++] = (uint8_t)strtoul(token, nullptr, 16);
        token = strtok(NULL, " ");
    }

    card.hasCard = (idx > 0);

    strncpy(card.name, filename, NFC_NAME_MAX);
    card.name[NFC_NAME_MAX] = '\0';
    char *dot = strrchr(card.name, '.');
    if (dot) *dot = '\0';

    file.close();
    return card.hasCard;
}

// ─────────────────────────────────────────────
// NFC LIST
// ─────────────────────────────────────────────
uint8_t listNFCFiles(char files[][NFC_NAME_MAX + 5], uint8_t maxFiles) {
    File root = SD.open("/");
    if (!root) return 0;

    uint8_t count = 0;

    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;

        String name = entry.name();
        if (name.endsWith(".nfc") && count < maxFiles) {
            strncpy(files[count], name.c_str(), NFC_NAME_MAX + 4);
            files[count][NFC_NAME_MAX + 4] = '\0';
            count++;
        }

        entry.close();
    }

    root.close();
    return count;
}

// ─────────────────────────────────────────────
// NFC DELETE
// ─────────────────────────────────────────────
bool deleteNFCFromSD(const char *filename) {
    char fullpath[40];
    snprintf(fullpath, sizeof(fullpath), "/%s", filename);
    return SD.remove(fullpath);
}