/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "keyboard.h"
#include "taphold.h"
#include "doubletap.h"
#include "combo.h"
#include "layers.h"
#include "macro.h"
#include "leds.h"
#include "matrix.h"

#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

// statics
static uint8_t* keyboard_hid_report_ref = NULL;
static uint8_t report_press_count = 0;

// externs
extern combo_t combos[NUM_COMBO_SLOTS];
extern macro_t macros[NUM_MACRO_SLOTS];
extern const keymap_entry_t keymap[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS];

// private functions
static void keyboard_handle_remaining_presses(void) {
    keymap_entry_t key = KC_NONE;

    // Now we have some certainty about the current layer, check for keypresses
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            key = keyboard_resolve_key(row, col);
            switch (key & ENTRY_TYPE_MASK) {
                case ENTRY_TYPE_KC: {
                    keyboard_send_key(key);

                    // this is a boot protocol keyboard, and it can't support more than 6 simultaneous keys.
                    // officially, we should be reporting an error through the hid protocol if there were more than 6, but let's skip
                    // that for now
                } break;
            }
        }
    }
}

static void keyboard_on_key_release(uint row, uint col, keymap_entry_t key) {
    if (kbc_on_key_release(row, col, key)) return;
    if (macro_on_key_release(row, col, key)) return;
    if (combo_on_key_release(row, col, key)) return;
    if (layers_on_key_release(row, col, key)) return;
    if (taphold_on_key_release(row, col, key)) return;
    if (double_tap_on_key_release(row, col, key)) return;
}

static void keyboard_on_key_press(uint row, uint col, keymap_entry_t key) {
    if (kbc_on_key_press(row, col, key)) return;
    if (macro_on_key_press(row, col, key)) return;

    if (!tapholds_any_active()) {
        if (combo_on_key_press(row, col, key)) return;
    }

    if (layers_on_key_press(row, col, key)) return;
    if (taphold_on_key_press(row, col, key)) return;
    if (double_tap_on_key_press(row, col, key)) return;
}

static void keyboard_handle_virtual_key(keymap_entry_t key) {
    if (kbc_on_virtual_key(key)) return;
    if (macro_on_virtual_key(key)) return;
    if (layers_on_virtual_key(key)) return;
}

static void keyboard_bootmagic(void) {
    gpio_put(matrix_get_col_gpio(BOOTMAGIC_COL), true);
    sleep_ms(1);
    if (gpio_get(matrix_get_row_gpio(BOOTMAGIC_ROW))) {
        reset_usb_boot(0, 0);
    }
    gpio_put(matrix_get_col_gpio(BOOTMAGIC_COL), false);
}

// public functions
void keyboard_init(uint8_t* keyboard_hid_report) {
    keyboard_hid_report_ref = keyboard_hid_report;

    // Reset to the bootrom if the escape key is held during boot
    keyboard_bootmagic();

    // Init tapholds
    taphold_init();

    // Init the double tap state
    double_tap_init();

    // Init combos
    combo_init(combos);

    // Init macros
    macro_init(macros);

    // Set the initial layer
    layers_set(LAYER_QWERTY);
}

bool keyboard_send_key(keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) != ENTRY_TYPE_KC) {
        keyboard_handle_virtual_key(key);
        return true;
    }

    if (report_press_count >= 6) return false;

    if (!keyboard_before_send_key(&key)) return false;

    uint8_t kc_value = key & KC_MASK;

    // All keys can now encode being held together with all possible modifiers
    keyboard_hid_report_ref[0] |= KEY_MODS(key);

    if (kc_value == KC_NONE) return true;

    // The key code value itself can be either a modifier or a key proper
    if ((kc_value >= KC_LCTL) && (kc_value <= KC_RGUI)) {
        // the bit position is encoded directly into the bottom nibble
        keyboard_hid_report_ref[0] |= (1 << (kc_value & 0xf));
    } else {
        // Avoid stuffing the same key in multiple times
        for (uint i = 2; i < 6; i++) {
            if (keyboard_hid_report_ref[i] == kc_value) return true;
        }
        keyboard_hid_report_ref[2 + report_press_count] = kc_value;
        ++report_press_count;
    }

    return true;
}

void keyboard_send_modifiers(uint8_t modifiers) {
    keyboard_hid_report_ref[0] |= modifiers;
}

void keyboard_clear_sent_keys(void) {
    memset(keyboard_hid_report_ref, 0, 8);
}

void keyboard_post_scan(void) {
    // Clear the report
    keyboard_clear_sent_keys();
    report_press_count = 0;

    // Before processing the keypresses, handle any released keys
    const uint32_t* released_bitmap = matrix_get_released_this_scan_bitmap();
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            if (released_bitmap[row] & (1 << col)) {
                keyboard_on_key_release(row, col, keyboard_resolve_key(row, col));
            }
        }
    }

    // Process the keys pressed
    const uint32_t* pressed_bitmap = matrix_get_pressed_this_scan_bitmap();
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            if (pressed_bitmap[row] & (1 << col)) {
                keyboard_on_key_press(row, col, keyboard_resolve_key(row, col));
            }
        }
    }

    if (!macro_update()) {
        // Handle combos before layer change operations to allow for the layer changing keys themselves to be used for combos
        bool ignore_remaining_keypresses = combo_update();

        // Tapholds
        ignore_remaining_keypresses = taphold_update() || ignore_remaining_keypresses;

        // Double taps
        ignore_remaining_keypresses = double_tap_update() || ignore_remaining_keypresses;

        // Regular keypresses that haven't been suppressed by other functionalities
        if (!ignore_remaining_keypresses) {
            keyboard_handle_remaining_presses();
        }
    }

    keyboard_on_scan_complete((const uint8_t*)keyboard_hid_report_ref);
}

keymap_entry_t keyboard_resolve_key(uint row, uint col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return KC_NONE;

    keymap_entry_t key = keymap[layers_get_current()][row][col];
    if (key == KC_TRANS) {
        key = keymap[layers_get_base()][row][col];
    }

    return key;
}

keymap_entry_t keyboard_resolve_key_on_layer(uint row, uint col, uint layer) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS || layer >= LAYER_MAX) return KC_NONE;

    keymap_entry_t key = keymap[layer][row][col];
    if (key == KC_TRANS) {
        key = keymap[layers_get_base()][row][col];
    }

    return key;
}

uint8_t keyboard_get_current_layer(void) {
    return layers_get_current();
}

// weak functions
__attribute__ ((weak)) bool kbc_on_key_press(uint row, uint col, keymap_entry_t key) {
    return false;
}

__attribute__ ((weak)) bool kbc_on_key_release(uint row, uint col, keymap_entry_t key) {
    return false;
}

__attribute__ ((weak)) bool kbc_on_virtual_key(keymap_entry_t key) {
    kbc_on_key_press(0xff, 0xff, key);
}

__attribute__ ((weak)) bool keyboard_before_send_key(keymap_entry_t* key) {
    return true;
}

__attribute__ ((weak)) void keyboard_on_led_status_report(uint8_t led_status) {

}

__attribute__ ((weak)) void keyboard_on_scan_complete(const uint8_t* hid_report) {

}
