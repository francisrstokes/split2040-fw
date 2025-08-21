/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrix.h"

#include <string.h>

#include "pico/stdlib.h"

// defines
#define LAYER_QWERTY    (0)
#define LAYER_LOWER     (1)
#define LAYER_RAISE     (2)
#define LAYER_FN        (3)

#define MAX_PRESSED_POSITIONS   (32)

#define LAYER_OPERATION_NONE    (0)
#define LAYER_OPERATION_MO      (1)

#define PRESS_PROCESSED         (0xff)

#define ____    KC_TRANS

#define LOWER   MO(LAYER_LOWER)
#define RAISE   MO(LAYER_RAISE)
#define FN      MO(LAYER_FN)

// typedefs
typedef struct pressed_pos_t {
    uint8_t col;
    uint8_t row;
} pressed_pos_t;

typedef struct layer_state_t {
    uint8_t base;
    uint8_t current;
    uint8_t operation;
} layer_state_t;

// statics
static uint matrix_cols[MATRIX_COLS] = { 5, 4, 3, 2, 1, 0, 20, 21, 22, 26, 27, 28 };
static uint matrix_rows[MATRIX_ROWS] = { 19, 18, 17, 16 };

static layer_state_t layer_state = {
    .base = LAYER_QWERTY,
    .current = LAYER_QWERTY,
    .operation = LAYER_OPERATION_NONE
};

static uint8_t press_count = 0;
static pressed_pos_t pressed_positions[MAX_PRESSED_POSITIONS] = {0};

static uint16_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_QWERTY] = {
        {KC_ESC,    KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       /* split */     KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,       KC_BSPC},
        {KC_TAB,    KC_A,       KC_S,       KC_D,       KC_F,       KC_G,       /* split */     KC_H,       KC_J,       KC_K,       KC_L,       KC_SCLN,    KC_QUOTE},
        {KC_LSFT,   KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       /* split */     KC_N,       KC_M,       KC_COMMA,   KC_DOT,     KC_SLASH,   KC_ENTER},
        {KC_LCTL,   KC_HOME,    KC_LALT,    KC_LGUI,    LOWER,      KC_SPC,     /* split */     KC_SPC,     RAISE,      KC_END,     KC_HOME,    KC_RSFT,    KC_RCTL}
    },

    [LAYER_LOWER] = {
        {KC_F1,    KC_F2,       KC_F3,     KC_F4,       KC_F5,     KC_F6,       /* split */     KC_F7,     KC_F8,       KC_F9,      KC_F10,     KC_F11,     KC_F12},
        {____,      KC_1,       KC_2,       KC_3,       KC_4,       KC_5,       /* split */     KC_6,      KC_7,        KC_8,       KC_9,       KC_0,       KC_MINUS},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,      KC_LEFT,     KC_DOWN,    KC_UP,      KC_RIGHT,   ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____}
    },

    [LAYER_RAISE] = {
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____}
    },

    [LAYER_FN] = {
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,       ____}
    }
};

// private functions
void matrix_handled_pressed_keys(uint8_t* keyboard_hid_report) {
    uint16_t key = KC_NONE;
    uint8_t kc_value = KC_NONE;

    uint8_t report_press_count = 0;

    // First scan for layer operations
    for (uint i = 0; i < press_count; i++) {
        key = keymap[layer_state.current][pressed_positions[i].row][pressed_positions[i].col];

        switch (key & ENTRY_TYPE_MASK) {
            case ENTRY_TYPE_LAYER: {
                switch (key & ENTRY_ARG_MASK) {
                    case LAYER_COM_MO: {
                        // A momentary layer switch is only active while the key is pressed, i.e. the state has to be re-evaluated every scan
                        layer_state.operation = LAYER_OPERATION_MO;
                        layer_state.current = key & KC_MASK;

                        // Don't process this entry on further operations
                        pressed_positions[i].col = PRESS_PROCESSED;
                    } break;
                }
            } break;

            // Skip everything else, since we can't know which layer to reference in the keymap
        }
    }

    // Now we have some certainty about the current layer, check for keypresses
    for (uint i = 0; i < press_count; i++) {
        // Skip any keys we've already processed
        if (pressed_positions[i].col == PRESS_PROCESSED) continue;

        // If this key is transparent, use the base layer instead
        key = keymap[layer_state.current][pressed_positions[i].row][pressed_positions[i].col];
        if (key == KC_TRANS) {
            key = keymap[layer_state.base][pressed_positions[i].row][pressed_positions[i].col];
        }
        kc_value = key & KC_MASK;

        switch (key & ENTRY_TYPE_MASK) {
            case ENTRY_TYPE_KC: {
                if ((kc_value >= KC_LCTL) && (kc_value <= KC_RGUI)) {
                    // the bit position is encoded directly into the bottom nibble
                    keyboard_hid_report[0] |= (1 << (kc_value & 0xf));
                } else {
                    keyboard_hid_report[2 + report_press_count] = kc_value;

                    // because this is a boot protocol keyboard, it can't support more than 6 simultaneous keys, so early exit.
                    // officially, we should be reporting an error through the hid protocol if there were more than 6, but let's skip
                    // that for now
                    if (++report_press_count >= 6) {
                        i = press_count;
                        break;
                    }
                }
            } break;
        }
    }

    // Once all the keys are processed, we can check if we need to do something with the layer state
    if (layer_state.operation == LAYER_OPERATION_MO) {
        layer_state.current = layer_state.base;
    }
}

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
    // Clear the report
    memset(keyboard_hid_report, 0, 8);

    // Clear the pressed key positions
    memset(pressed_positions, 0, sizeof(pressed_positions));
    press_count = 0;

    // Scan each column in turn, reading back the rows
    for (uint col = 0; col < MATRIX_COLS; col++) {
        // Assert the column
        gpio_put(matrix_cols[col], true);

        // Scan the rows
        for (uint row = 0; row < MATRIX_ROWS; row++) {
            if (gpio_get(matrix_rows[row])) {
                pressed_positions[press_count].col = col;
                pressed_positions[press_count].row = row;
                if (++press_count == MAX_PRESSED_POSITIONS) {
                    gpio_put(matrix_cols[col], false);

                    // Breaking out of double loops is annoying, let's use Dijkstra's favourite programming construct
                    goto early_scan_stop;
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
    early_scan_stop:

    // Once the scan is complete, process the pressed keys, which could have functions beyond standard keypresses
    matrix_handled_pressed_keys(keyboard_hid_report);
}
