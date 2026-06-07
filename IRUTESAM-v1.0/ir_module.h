#ifndef IR_MODULE_H
#define IR_MODULE_H

#include <Arduino.h>
#include "config.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

struct IRSignal {
    decode_type_t protocol;
    uint64_t      value;
    uint16_t      bits;
    uint32_t      freq;
    uint16_t      rawData[IR_MAX_RAW];
    uint16_t      rawLen;
    char          name[IR_NAME_MAX + 1];
    bool          hasSignal;
};

void     irModuleInit();
void     irStartCapture();
void     irStopCapture();
bool     irPollCapture(IRSignal &sig);
bool     irSendSignal(const IRSignal &sig);
uint32_t irGetFreq(decode_type_t proto);
String   irProtoName(decode_type_t proto);
void     irClearSignal(IRSignal &sig);

extern IRSignal capturedIR;
extern IRSignal selectedIR;

#endif