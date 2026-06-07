#include "ir_module.h"

#ifndef ESP32
#error "Select ESP32 board in Arduino IDE"
#endif

// ── IR objects ───────────────────────────────────────────────
static IRrecv irrecv(IR_RX_PIN, IR_CAPTURE_BUF, IR_TIMEOUT_MS, true);
static IRsend irsend(IR_TX_PIN);
static decode_results results;

static bool captureEnabled = false;

IRSignal capturedIR;
IRSignal selectedIR;

// ─────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────
void irClearSignal(IRSignal &sig) {
    memset(&sig, 0, sizeof(IRSignal));
    sig.protocol  = UNKNOWN;
    sig.value     = 0;
    sig.bits      = 0;
    sig.freq      = 38000;
    sig.rawLen    = 0;
    sig.name[0]   = '\0';
    sig.hasSignal = false;
}

uint32_t irGetFreq(decode_type_t proto) {
    switch (proto) {
        case SONY: return 40000;
        case RC5:
        case RC6:  return 36000;
        default:   return 38000;
    }
}

String irProtoName(decode_type_t proto) {
    return typeToString(proto);
}

// ─────────────────────────────────────────────────────────────
// Init / capture control
// ─────────────────────────────────────────────────────────────
void irModuleInit() {
    irClearSignal(capturedIR);
    irClearSignal(selectedIR);

    irsend.begin();
    irrecv.enableIRIn();
    captureEnabled = true;

    Serial.println(F("[IR] Module ready"));
    Serial.print(F("[IR] RX pin: "));
    Serial.println(IR_RX_PIN);
    Serial.print(F("[IR] TX pin: "));
    Serial.println(IR_TX_PIN);
}

void irStartCapture() {
    irrecv.enableIRIn();
    captureEnabled = true;
}

void irStopCapture() {
    irrecv.disableIRIn();
    captureEnabled = false;
}

// ─────────────────────────────────────────────────────────────
// Capture
// ─────────────────────────────────────────────────────────────
bool irPollCapture(IRSignal &sig) {
    if (!captureEnabled) return false;
    if (!irrecv.decode(&results)) return false;

    irClearSignal(sig);

    sig.protocol  = results.decode_type;
    sig.value     = results.value;
    sig.bits      = results.bits;
    sig.freq      = irGetFreq(results.decode_type);
    sig.hasSignal = true;

    // IMPORTANT:
    // rawbuf[] is in ticks, convert to microseconds like your old working code
    uint16_t safeLen = 0;
    if (results.rawlen > 1) {
        safeLen = min((uint16_t)(results.rawlen - 1), (uint16_t)IR_MAX_RAW);
    }

    sig.rawLen = safeLen;
    for (uint16_t i = 0; i < safeLen; i++) {
        sig.rawData[i] = results.rawbuf[i + 1] * kRawTick;
    }

    strncpy(sig.name, "CAPTURED", IR_NAME_MAX);
    sig.name[IR_NAME_MAX] = '\0';

    capturedIR = sig;

    Serial.println();
    Serial.println(F("========== IR CAPTURE =========="));
    Serial.print(F("Protocol : "));
    Serial.println(typeToString(sig.protocol));
    Serial.print(F("Bits     : "));
    Serial.println(sig.bits);

    if (sig.protocol != UNKNOWN) {
        Serial.print(F("Value    : 0x"));
        Serial.println((uint32_t)(sig.value & 0xFFFFFFFF), HEX);
    } else {
        Serial.println(F("Value    : UNKNOWN"));
    }

    Serial.print(F("RawLen   : "));
    Serial.println(sig.rawLen);
    Serial.print(F("Freq     : "));
    Serial.println(sig.freq);
    Serial.println(F("================================"));

    irrecv.resume();
    return true;
}

// ─────────────────────────────────────────────────────────────
// Send helpers
// ─────────────────────────────────────────────────────────────
static bool sendDecodedSignal(const IRSignal &sig) {
    bool ok = true;

    switch (sig.protocol) {
        case NEC:
            irsend.sendNEC(sig.value, sig.bits);
            break;

        case SAMSUNG:
            irsend.sendSAMSUNG(sig.value, sig.bits);
            break;

        case SAMSUNG36:
            irsend.sendSamsung36(sig.value, sig.bits);
            break;

        case LG:
            irsend.sendLG(sig.value, sig.bits);
            break;

        case LG2:
            irsend.sendLG2(sig.value, sig.bits);
            break;

        case SONY:
            // Sony usually needs repeats
            irsend.sendSony(sig.value, sig.bits);
            delay(45);
            irsend.sendSony(sig.value, sig.bits);
            delay(45);
            irsend.sendSony(sig.value, sig.bits);
            break;

        case RC5:
            irsend.sendRC5(sig.value, sig.bits);
            break;

        case RC6:
            irsend.sendRC6(sig.value, sig.bits);
            break;

        case PANASONIC:
            irsend.sendPanasonic(sig.bits, sig.value);
            break;

        case JVC:
            irsend.sendJVC(sig.value, sig.bits, 0);
            break;

        case DENON:
            irsend.sendDenon(sig.value, sig.bits);
            break;

        case DISH:
            irsend.sendDISH(sig.value, sig.bits);
            break;

        case SHARP:
            irsend.sendSharpRaw(sig.value, sig.bits);
            break;

        case MITSUBISHI:
            irsend.sendMitsubishi(sig.value, sig.bits);
            break;

        case PIONEER:
            irsend.sendPioneer(sig.value, sig.bits, 0);
            break;

        default:
            ok = false;
            break;
    }

    return ok;
}

static bool sendRawSignal(const IRSignal &sig) {
    if (sig.rawLen == 0) return false;

    uint16_t khz = sig.freq / 1000;
    if (khz == 0) khz = 38;

    for (uint8_t i = 0; i < 3; i++) {
        irsend.sendRaw((uint16_t*)sig.rawData, sig.rawLen, khz);
        delay(100);
    }
    return true;
}

// ─────────────────────────────────────────────────────────────
// Send
// ─────────────────────────────────────────────────────────────
bool irSendSignal(const IRSignal &sig) {
    if (!sig.hasSignal) {
        Serial.println(F("[IR] No signal to send"));
        return false;
    }

    Serial.println(F("[IR] Sending..."));

    // stop receiver while transmitting
    irrecv.disableIRIn();

    bool ok = false;

    // 1) Try protocol-aware sending first
    if (sig.protocol != UNKNOWN) {
        ok = sendDecodedSignal(sig);
        if (ok) {
            Serial.println(F("[IR] Sent using protocol"));
        }
    }

    // 2) Raw fallback
    if (!ok) {
        ok = sendRawSignal(sig);
        if (ok) {
            Serial.println(F("[IR] Sent using raw fallback"));
        }
    }

    // restart receiver
    irrecv.enableIRIn();

    if (!ok) {
        Serial.println(F("[IR] Send failed"));
    }

    return ok;
}