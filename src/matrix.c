/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrix.h"

#include <string.h>

#include "pico/stdlib.h"

// statics
static uint matrix_cols[MATRIX_COLS] = { 5, 4, 3, 2, 1, 0, 20, 21, 22, 26, 27, 28 };
static uint matrix_rows[MATRIX_ROWS] = { 19, 18, 17, 16 };

static uint32_t keymap[MATRIX_ROWS][MATRIX_COLS] = {
    {KC_ESC,    KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       /* split */     KC_Y,   KC_U,       KC_I,       KC_O,       KC_P,       KC_BSPC},
    {KC_TAB,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       /* split */     KC_H,   KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOTE},
    {KC_LSFT,   KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       /* split */     KC_N,   KC_M,       KC_COMMA,   KC_DOT,     KC_SLASH,   KC_ENTER},
    {KC_LCTL,   KC_HOME,    KC_LALT,    KC_LGUI,    KC_NONE,    KC_SPC,     /* split */     KC_SPC, KC_NONE,    KC_END,     KC_HOME,    KC_RSFT,    KC_RCTL}
};

// public functions
void matrix_init(void) {
    // Scan asserts a high on a column and reads back the rows
    for (uint i = 0; i < MATRIX_COLS; i++) {
        gpio_init(matrix_cols[i]);
        gpio_set_dir(matrix_cols[i], GPIO_OUT);
        gpio_put(matrix_cols[i], false);
    }

    // Rows are pulled down
    for (uint i = 0; i < MATRIX_ROWS; i++) {
        gpio_init(matrix_rows[i]);
        gpio_set_dir(matrix_rows[i], GPIO_IN);
        gpio_pull_down(matrix_rows[i]);
    }
}

void matrix_scan(uint8_t* keyboard_hid_report) {
    uint8_t pressed_count = 0;
    uint32_t key = KC_NONE;
    uint8_t kc_value = KC_NONE;

    // Clear the report
    memset(keyboard_hid_report, 0, 8);

    // Scan each column in turn, reading back the rows
    for (uint col = 0; col < MATRIX_COLS; col++) {
        // Assert the column
        gpio_put(matrix_cols[col], true);

        // Scan the rows
        for (uint row = 0; row < MATRIX_ROWS; row++) {
            if (gpio_get(matrix_rows[row])) {
                key = keymap[row][col];
                kc_value = key & KC_MASK;

                // key is pressed.  if it's a meta key, it doesn't get recorded in the regular array
                if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_KC) {
                    if ((kc_value >= KC_LCTL) && (kc_value <= KC_RGUI)) {
                        // the bit position is encoded directly into the bottom nibble
                        keyboard_hid_report[0] |= (1 << (kc_value & 0xf));
                    } else {
                        keyboard_hid_report[2 + pressed_count] = kc_value;

                        // because this is a boot protocol keyboard, it can't support more than 6 simultaneous keys, so early exit.
                        // officially, we should be reporting an error through the hid protocol if there were more than 6, but let's skip
                        // that for now
                        if (++pressed_count >= 6) {
                            gpio_put(matrix_cols[col], false);
                            return;
                        }
                    }
                }
            }
        }

        // Deassert the column
        gpio_put(matrix_cols[col], false);

        // Wait some time to avoid asserting multiple columns at the same time
        for (uint i = 0; i < 25; i++) {
            asm volatile("nop\n");
        }
    }
}
