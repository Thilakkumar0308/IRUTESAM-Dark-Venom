#ifndef TOUCH_INPUT_H
#define TOUCH_INPUT_H

#include <Arduino.h>
#include "config.h"

// ── Touch event types ────────────────────────────────────────
enum TouchEvent {
    TOUCH_NONE,
    TOUCH_UP,        // tapped top third of content area
    TOUCH_DOWN,      // tapped bottom third of content area
    TOUCH_SELECT,    // single tap in centre third
    TOUCH_OPEN,      // double-tap OR hold in centre third
    TOUCH_RAW        // raw positional tap (for non-menu screens)
};

void        touchInit();
TouchEvent  touchPoll();          // replaces bool touchPoll()
bool        touchRaw();           // returns true if any tap fired this frame
int         getLastTouchX();
int         getLastTouchY();
bool        isTouched();
void        touchClearState();
TouchEvent  getLastEvent();

#endif
