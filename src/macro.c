#include "macro.h"
#include "keyboard.h"
#include "matrix.h"

// statics
static macro_t* macros = NULL;
static const uint8_t ascii_to_hid_kc[128][2] =  { HID_ASCII_TO_KEYCODE };
static bool any_macro_active = false;

// private functions
static void macro_start(uint index) {
    if (index < MACRO_MAX && macros[index].type != macro_type_unused) {
        any_macro_active = true;
        macros[index].active = true;
        switch (macros[index].type) {
            case macro_type_send_string: {
                macros[index].send_string.index = 0;
            } break;
        }
    }
}

static void macro_check_any_active(void) {
    for (uint macro_index = 0; macro_index < MACRO_MAX; macro_index++) {
        if (macros[macro_index].type == macro_type_unused) break;
        if (macros[macro_index].active) {
            any_macro_active = true;
            return;
        }
    }
    any_macro_active = false;
}

// public functions
void macro_init(macro_t* macro_table) {
    macros = macro_table;
}

bool macro_on_key_press(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_MACRO) {
        macro_start(key & MACRO_INDEX_MASK);
        return true;
    }
    return false;
}

bool macro_on_key_release(uint row, uint col, keymap_entry_t key) {
    return false;
}

bool macro_on_virtual_key(keymap_entry_t key) {
    macro_on_key_press(0xff, 0xff, key);
}

bool macro_update(void) {
    bool cleared_sent_keys = false;

    for (uint macro_index = 0; macro_index < MACRO_MAX; macro_index++) {
        if (macros[macro_index].type == macro_type_unused) break;
        if (!macros[macro_index].active) break;

        switch (macros[macro_index].type) {
            case macro_type_send_string: {
                const uint32_t buf_index = macros[macro_index].send_string.index;
                const uint8_t key_code = ascii_to_hid_kc[macros[macro_index].send_string.buffer[buf_index]][1];
                const uint8_t modifier = (ascii_to_hid_kc[macros[macro_index].send_string.buffer[buf_index]][0] == 1) ? (1 << 1) : 0;

                // Ensure the HID buffer is empty
                if (buf_index == 0 && !cleared_sent_keys) {
                    keyboard_clear_sent_keys();
                    cleared_sent_keys = true;
                }

                keyboard_send_key(key_code | (modifier << 8));

                if (++macros[macro_index].send_string.index >= (macros[macro_index].send_string.length - 1)) {
                    macros[macro_index].active = false;
                    macro_check_any_active();
                }
            } break;
        }
    }

    return any_macro_active;
}

bool macro_any_active(void) {
    return any_macro_active;
}
