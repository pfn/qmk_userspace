#include QMK_KEYBOARD_H

enum custom_keycodes {
    DRAG_SCROLL = SAFE_RANGE,
};

extern uint8_t numpad_layer;
extern uint8_t sym_layer;
extern uint8_t mouse_layer;
extern uint8_t mod_held;
extern bool is_scrolling;