#include "sd_manager.h"

#include <SPI.h>

SPIClass sdSPI(HSPI);  // 🔥 IMPORTANT

bool sdInit() {

    sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    if (!SD.begin(SD_CS_PIN, sdSPI)) {
        Serial.println("SD FAIL");
        return false;
    }

    Serial.println("SD OK");
    return true;
}

// SAVE
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

    file.close();
    return true;
}

// LOAD
bool loadIRFromSD(const char *filename, IRSignal &sig) {

    char fullpath[40];
    snprintf(fullpath, sizeof(fullpath), "/%s", filename);

    File file = SD.open(fullpath);

    // ── ADD THIS GUARD ───────────────────────────────────────
    if (!file) {
        Serial.print("[SD] File not found: ");
        Serial.println(fullpath);
        sig.hasSignal = false;
        return false;
    }

    irClearSignal(sig);             // ← also add this; clears stale data

    String header = file.readStringUntil('\n');
    header.trim();

    sscanf(header.c_str(), "%u,%llu,%u,%u",
        (unsigned int*)&sig.protocol,
        &sig.value,
        &sig.bits,
        &sig.rawLen);

    if (sig.rawLen > IR_MAX_RAW) sig.rawLen = IR_MAX_RAW;   // ← bounds check

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

    // Copy name from filename (strip .ir extension)
    strncpy(sig.name, filename, IR_NAME_MAX);
    sig.name[IR_NAME_MAX] = '\0';
    char *dot = strrchr(sig.name, '.');
    if (dot) *dot = '\0';

    return true;
}

// LIST FILES
uint8_t listIRFiles(char files[][IR_NAME_MAX + 5], uint8_t maxFiles) {

    File root = SD.open("/");
    uint8_t count = 0;

    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;

        String name = entry.name();

        if (name.endsWith(".ir") && count < maxFiles) {
            strncpy(files[count], name.c_str(), IR_NAME_MAX + 4);
            count++;
        }

        entry.close();
    }

    return count;
}

// DELETE
bool deleteIRFromSD(const char *filename) {
    return SD.remove(filename);
}
