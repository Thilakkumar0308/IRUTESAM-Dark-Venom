#include "nav_manager.h"
#include "touch_input.h"

NavManager nav;

NavManager::NavManager() : _top(0), _newScreen(true) {
    memset(_stack, 0, sizeof(_stack));
    _stack[0].scr    = SCR_BOOT;
    _stack[0].sel    = 0;
    _stack[0].offset = 0;
}

void NavManager::goTo(Screen scr, uint8_t initSel, uint8_t initOffset) {
    if (_top < NAV_STACK_DEPTH - 1) {
        _top++;
    }
    _stack[_top].scr    = scr;
    _stack[_top].sel    = initSel;
    _stack[_top].offset = initOffset;
    _newScreen = true;
    touchClearState();
}

bool NavManager::goBack() {
    if (_top == 0) return false;
    _top--;
    _newScreen = true;
    touchClearState();
    return true;
}

void NavManager::replace(Screen scr) {
    _stack[_top].scr    = scr;
    _stack[_top].sel    = 0;
    _stack[_top].offset = 0;
    _newScreen = true;
    touchClearState();
}
