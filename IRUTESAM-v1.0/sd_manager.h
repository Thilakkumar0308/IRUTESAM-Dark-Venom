#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "ir_module.h"
#include "nfc_module.h"
#include <SD.h>

bool sdInit();

bool saveIRToSD(const IRSignal &sig);
bool loadIRFromSD(const char *filename, IRSignal &sig);
uint8_t listIRFiles(char files[][IR_NAME_MAX + 5], uint8_t maxFiles);
bool deleteIRFromSD(const char *filename);

bool saveNFCToSD(const NFCCard &card);
bool loadNFCFromSD(const char *filename, NFCCard &card);
uint8_t listNFCFiles(char files[][NFC_NAME_MAX + 5], uint8_t maxFiles);
bool deleteNFCFromSD(const char *filename);

#endif