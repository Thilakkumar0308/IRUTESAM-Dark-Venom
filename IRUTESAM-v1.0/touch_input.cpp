#include "touch_input.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

static const int TS_MINX = 404;
static const int TS_MAXX = 3531;
static const int TS_MINY = 688;
static const int TS_MAXY = 3442;

static const unsigned long DEBOUNCE_MS      = 80;
static const unsigned long DOUBLE_TAP_MS    = 350;
static const unsigned long HOLD_MS          = 600;
static const unsigned long POLL_INTERVAL_MS = 20;

static const int TOUCH_Z_MIN = 150;
static const int TOUCH_Z_MAX = 4000;

#define CONTENT_TOP   STATUS_H
#define CONTENT_BOT   FOOTER_Y
#define CONTENT_H     (CONTENT_BOT - CONTENT_TOP)
#define ZONE_TOP_MAX  (CONTENT_TOP + CONTENT_H / 3)
#define ZONE_BOT_MIN  (CONTENT_BOT - CONTENT_H / 3)

static XPT2046_Touchscreen ts(TOUCH_CS_PIN, 255);

static int           _lastX       = -1;
static int           _lastY       = -1;
static bool          _physTouched = false;
static unsigned long _pressStart  = 0;
static bool          _holdFired   = false;

static unsigned long _lastTapMs   = 0;
static int           _lastTapY    = -1;
static bool          _pendingTap  = false;

static unsigned long _lastPollMs  = 0;

static TouchEvent    _lastEvent   = TOUCH_NONE;
static bool          _rawFired    = false;

static inline void spiReleaseAll() {
    digitalWrite(TFT_CS_PIN,   HIGH);
    digitalWrite(TOUCH_CS_PIN, HIGH);
    digitalWrite(SD_CS_PIN,    HIGH);
}

static inline void spiSelectTouch() {
    digitalWrite(TFT_CS_PIN,   HIGH);
    digitalWrite(SD_CS_PIN,    HIGH);
    digitalWrite(TOUCH_CS_PIN, LOW);
}

static TouchEvent classifyZone(int y) {
    if (y < CONTENT_TOP || y >= CONTENT_BOT) return TOUCH_RAW;
    if (y < ZONE_TOP_MAX)  return TOUCH_UP;
    if (y >= ZONE_BOT_MIN) return TOUCH_DOWN;
    return TOUCH_SELECT;
}

void touchInit() {
    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH);
    spiReleaseAll();

    if (!ts.begin()) {
        touchAvailable = false;
        return;
    }

    ts.setRotation(2);
    touchAvailable = true;
    _physTouched   = false;
    _lastX         = -1;
    _lastY         = -1;
    _lastTapMs     = 0;      // ← fixed: was _lastTouchMs
    _lastPollMs    = 0;
    _lastEvent     = TOUCH_NONE;
    _rawFired      = false;
    _pendingTap    = false;
    _holdFired     = false;
    _pressStart    = 0;
    _lastTapY      = -1;
}

TouchEvent touchPoll() {
    _lastEvent = TOUCH_NONE;
    _rawFired  = false;

    if (!touchAvailable) return TOUCH_NONE;

    unsigned long now = millis();
    if (now - _lastPollMs < POLL_INTERVAL_MS) return TOUCH_NONE;
    _lastPollMs = now;

    spiSelectTouch();
    bool fingerDown = ts.touched();
    digitalWrite(TOUCH_CS_PIN, HIGH);

    // ── Finger just lifted ───────────────────────────────────
    if (!fingerDown && _physTouched) {
        _physTouched = false;
        _holdFired   = false;

        if (_pendingTap) {
            unsigned long gap = now - _lastTapMs;
            if (gap <= DOUBLE_TAP_MS &&
                abs(_lastY - _lastTapY) < (CONTENT_H / 3)) {
                _pendingTap = false;
                _lastEvent  = TOUCH_OPEN;
                _rawFired   = true;
                return TOUCH_OPEN;
            }
            // Window expired — emit first tap
            _pendingTap = false;
            TouchEvent z = classifyZone(_lastTapY);
            _lastEvent = z;
            _rawFired  = true;
            return z;
        }

        // First tap lifted — start double-tap window
        TouchEvent z = classifyZone(_lastY);
        if (z == TOUCH_SELECT) {
            // Park it as pending; wait for possible second tap
            _pendingTap = true;
            _lastTapMs  = now;
            _lastTapY   = _lastY;
            return TOUCH_NONE;
        }
        // UP / DOWN / RAW — emit immediately on lift
        _lastEvent = z;
        _rawFired  = true;
        return z;
    }

    // ── Pending double-tap timeout (no finger) ───────────────
    if (!fingerDown && _pendingTap) {
        if (now - _lastTapMs > DOUBLE_TAP_MS) {
            _pendingTap = false;
            TouchEvent z = classifyZone(_lastTapY);
            _lastEvent = z;
            _rawFired  = true;
            return z;
        }
        return TOUCH_NONE;
    }

    if (!fingerDown) return TOUCH_NONE;

    // ── New press ────────────────────────────────────────────
    if (!_physTouched) {
        spiSelectTouch();
// ───── MULTI-SAMPLE FILTER ─────
int samples = 5;
long sumX = 0, sumY = 0;
int valid = 0;

for (int i = 0; i < samples; i++) {
    TS_Point p = ts.getPoint();

    if (p.z > TOUCH_Z_MIN && p.z < TOUCH_Z_MAX) {
        sumX += p.x;
        sumY += p.y;
        valid++;
    }
    delay(2); // small delay improves stability
}

if (valid == 0) return TOUCH_NONE;

int rawX = sumX / valid;
int rawY = sumY / valid;

// ───── EDGE CORRECTION (IMPORTANT) ─────
rawX = constrain(rawX, TS_MINX + 10, TS_MAXX - 10);
rawY = constrain(rawY, TS_MINY + 10, TS_MAXY - 10);

// ───── FINAL MAPPING ─────
// AFTER averaging (keep your averaging code if added)

int x = map(rawX, TS_MINX, TS_MAXX, 0, SCREEN_W);
int y = map(rawY, TS_MINY - 15, TS_MAXY + 10, 0, SCREEN_H);

// ───── OFFSET FIX (MAIN FIX) ─────
x += 2;   // small horizontal adjust
y += 10;  // 🔥 MAIN vertical fix

// ───── FINAL LIMIT ─────
x = constrain(x, 0, SCREEN_W - 1);
y = constrain(y, 0, SCREEN_H - 1);

        _lastX       = x;
        _lastY       = y;
        _physTouched = true;
        _pressStart  = now;
        _holdFired   = false;
        return TOUCH_NONE;
    }

    // ── Finger held — check hold threshold ───────────────────
    if (_physTouched && !_holdFired) {
        unsigned long held = now - _pressStart;
        if (held >= HOLD_MS) {
            _holdFired  = true;
            _pendingTap = false;
            TouchEvent z = classifyZone(_lastY);
            if (z == TOUCH_SELECT) {
                _lastEvent = TOUCH_OPEN;
                _rawFired  = true;
                return TOUCH_OPEN;
            }
            // Hold in UP/DOWN zone — emit the scroll once
            _lastEvent = z;
            _rawFired  = true;
            return z;
        }
    }

    return TOUCH_NONE;
}

bool touchRaw()           { return _rawFired; }
int  getLastTouchX()      { return _lastX; }
int  getLastTouchY()      { return _lastY; }
bool isTouched()          { return _physTouched; }
TouchEvent getLastEvent() { return _lastEvent; }

void touchClearState() {
    _physTouched = false;
    _pendingTap  = false;
    _holdFired   = false;
    _lastX       = -1;
    _lastY       = -1;
    _pressStart  = 0;
    _lastTapMs   = 0;
    _lastTapY    = -1;
    _lastEvent   = TOUCH_NONE;
    _rawFired    = false;
}