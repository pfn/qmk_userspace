#include QMK_KEYBOARD_H

#define NUMLOCK_TIMEOUT 800

uint8_t numpad_layer = 1;
uint8_t sym_layer = 2;

uint16_t num_lock_timer = 0;
uint8_t mod_held = KC_NO;

bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case RCTL_T(KC_QUOT):
        // these for chocofi
        case RCTL_T(KC_SCLN):
        case RCTL_T(KC_O): // colemak
            return true;
        // these for 3x5 keebs
        case LSFT_T(KC_Z):
            return true;
        default:
            return false;
    }
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    if (keycode == LT(numpad_layer, KC_BSPC) || keycode == LT(sym_layer, KC_ESC))
        return true;

    switch (keycode) {
        case KC_ESC:
        case KC_BSPC:
        case KC_MPRV:
        case KC_MNXT:
        case LCTL_T(KC_TAB):
        case RSFT_T(KC_SLSH):
            return true;
        default:
            return false;
    }
}

// looks like reappropriating existing keycodes crashes qmk, don't do this...
//qk_tap_dance_action_t tap_dance_actions[] = {
//    [KC_F13] = ACTION_TAP_DANCE_DOUBLE(KC_VOLD, KC_MUTE)
//};

// caps word ported from https://github.com/precondition/dactyl-manuform-keymap/blob/main/keymap.c
static bool caps_word_on = false;
void caps_word_enable(void) {
    caps_word_on = true;
    if (!(host_keyboard_led_state().caps_lock)) {
        tap_code(KC_CAPS);
    }
}

void caps_word_disable(void) {
    caps_word_on = false;
    // why was unregistering shift a thing?
    // unregister_mods(MOD_MASK_SHIFT);
    if (host_keyboard_led_state().caps_lock) {
        tap_code(KC_CAPS);
    }
}

#define GET_TAP_KC(dual_role_key) dual_role_key & 0xFF

void process_caps_word(uint16_t keycode, const keyrecord_t *record) {
    // Update caps word state
    if (caps_word_on) {
        switch (keycode) {
            case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
                // Earlier return if this has not been considered tapped yet
                if (record->tap.count == 0) { return; }
                // Get the base tapping keycode of a mod- or layer-tap key
                keycode = GET_TAP_KC(keycode);
                break;
            default:
                break;
        }

        switch (keycode) {
            // prevent canceling caps word on layer shift keys that wouldn't otherwise cancel
            case QK_MOMENTARY ... QK_MOMENTARY_MAX:
            // keep shift-spc from cancelling caps word
            case KC_LSFT:
            case KC_RSFT:
                break;
            case KC_SPC:
                if (!(get_mods() & MOD_MASK_SHIFT)) {
                    caps_word_disable();
                }
                break;
            // Keycodes to shift
            case KC_A ... KC_Z:
                if (record->event.pressed) {
                    if (get_oneshot_mods() & MOD_MASK_SHIFT) {
                        caps_word_disable();
                        add_oneshot_mods(MOD_MASK_SHIFT);
                    } else {
                        caps_word_enable();
                    }
                }
            // Keycodes that enable caps word but shouldn't get shifted
            case KC_MINS:
            case KC_BSPC:
            case KC_UNDS:
            case KC_PIPE:
            case KC_CAPS:
            case KC_LPRN:
            case KC_RPRN:
                // If chording mods, disable caps word
                if (record->event.pressed && (get_mods() != MOD_LSFT) && (get_mods() != 0)) {
                    caps_word_disable();
                }
                break;
            default:
                // Any other keycode should automatically disable caps
                if (record->event.pressed && !(get_oneshot_mods() & MOD_MASK_SHIFT)) {
                    caps_word_disable();
                }
                break;
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (mod_held != KC_NO) {
        unregister_code(mod_held);
        mod_held = KC_NO;
    }

    process_caps_word(keycode, record);
    uint8_t mod_state = get_mods();
    switch (keycode) {
    case KC_PDOT:
    case KC_PPLS:
    case KC_PMNS:
    case KC_PSLS:
    case KC_PAST:
    case KC_EQL: // KC_PEQL is not usable?
    case KC_1 ... KC_0:
        if (record->event.pressed) {
            register_code(keycode);
            if (get_oneshot_layer() == numpad_layer) {
                num_lock_timer = timer_read();
            }
        } else {
            unregister_code(keycode);
        }
        return false;
        break;
    case KC_CAPS: // CAPS_WORD
        // Toggle `caps_word_on`
        if (record->event.pressed) {
            if (caps_word_on) {
                caps_word_disable();
                return false;
            } else {
                caps_word_enable();
                return false;
            }
        }
        break;


    /*
    case KC_SPC: // shift-spc = _
        if (mod_state & MOD_MASK_SHIFT) {
            if (record->event.pressed) {
                tap_code(KC_MINS);
            }
            return false;
        }
        return true;
    */

    case KC_BSPC: // shift-bspc = del
        {
            static bool delkey_registered;
            if (record->event.pressed) {
                if (mod_state & MOD_MASK_SHIFT) {
                    if (!(mod_state & MOD_BIT(KC_LSHIFT)) != !(mod_state & MOD_BIT(KC_RSHIFT))) {
                        del_mods(MOD_MASK_SHIFT);
                    }
                    register_code(KC_DEL);
                    delkey_registered = true;
                    set_mods(mod_state);
                    return false;
                }
            } else {
                if (delkey_registered) {
                    unregister_code(KC_DEL);
                    delkey_registered = false;
                    return false;
                }
            }
            return true;
        }
    }
    if (get_oneshot_layer() == numpad_layer) {
        num_lock_timer = 0;
        // make sure to turn off the layer, doesn't always get turned off
        clear_oneshot_layer_state(ONESHOT_PRESSED);
    }

    return true;
};

void oneshot_layer_changed_user(uint8_t layer) {
    // set oneshot layer back on when it gets turned off by keypress
    if (!layer && num_lock_timer) {
        set_oneshot_layer(numpad_layer, ONESHOT_START);
    }
}

void matrix_scan_user(void) {
    // time out the numpad osl
    if (num_lock_timer && timer_elapsed(num_lock_timer) > NUMLOCK_TIMEOUT) {
        num_lock_timer = 0;
        clear_oneshot_layer_state(ONESHOT_PRESSED);
    }
}
