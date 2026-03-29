#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "ir_module.h"
#include <SD.h>

bool sdInit();

bool saveIRToSD(const IRSignal &sig);
bool loadIRFromSD(const char *filename, IRSignal &sig);
uint8_t listIRFiles(char files[][IR_NAME_MAX + 5], uint8_t maxFiles);
bool deleteIRFromSD(const char *filename);

#endif
