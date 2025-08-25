/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "keyboard.h"
#include "matrix.h"
#include "ll_alloc.h"

#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

// options/configuration
#define TAP_HOLD_DELAY_MS           (200)
#define MAX_CONCURRENT_TAPHOLDS     (8)

#define BOOTMAGIC_COL               (0)
#define BOOTMAGIC_ROW               (0)

// regular defines
#define LAYER_QWERTY            (0)
#define LAYER_LOWER             (1)
#define LAYER_RAISE             (2)
#define LAYER_FN                (3)

#define LAYER_OPERATION_NONE    (0)
#define LAYER_OPERATION_MO      (1)

#define PRESS_PROCESSED         (0xff)

#define ____                    KC_TRANS

#define LOWER                   MO(LAYER_LOWER)
#define RAISE                   MO(LAYER_RAISE)
#define FN                      MO(LAYER_FN)

#define C_LEFT                  LC(KC_LEFT)
#define C_DOWN                  LC(KC_DOWN)
#define C_UP                    LC(KC_UP)
#define C_RIGHT                 LC(KC_RIGHT)

#define S_1                     LS(KC_1)
#define S_2                     LS(KC_2)
#define S_3                     LS(KC_3)
#define S_4                     LS(KC_4)
#define S_5                     LS(KC_5)
#define S_6                     LS(KC_6)
#define S_7                     LS(KC_7)
#define S_8                     LS(KC_8)
#define S_9                     LS(KC_9)
#define S_0                     LS(KC_0)
#define S_MINUS                 LS(KC_MINUS)

// typedefs
typedef struct layer_state_t {
    uint8_t base;
    uint8_t current;
    uint8_t operation;
} layer_state_t;

typedef struct taphold_data_t {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
    uint8_t hold_counter;
} taphold_data_t;

typedef struct taphold_state_t {
    taphold_data_t data_array[MAX_CONCURRENT_TAPHOLDS];
    ll_node_t node_array[MAX_CONCURRENT_TAPHOLDS];
    ll_allocator_t allocator;
} taphold_state_t;

// statics
static layer_state_t layer_state = {
    .base = LAYER_QWERTY,
    .current = LAYER_QWERTY,
    .operation = LAYER_OPERATION_NONE
};

static uint8_t* keyboard_hid_report_ref = NULL;
static uint8_t report_press_count = 0;
static taphold_state_t tapholds = {0};

static uint16_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_QWERTY] = {
        {KC_ESC,    KC_Q,       KC_W,       KC_E,           KC_R,           KC_T,       /* split */     KC_Y,       KC_U,           KC_I,       KC_O,       KC_P,           KC_BSPC},
        {KC_TAB,    LG_T(KC_A), LA_T(KC_S), LS_T(KC_D),     LC_T(KC_F),     KC_G,       /* split */     KC_H,       LC_T(KC_J),     LS_T(KC_K), LA_T(KC_L), LG_T(KC_SCLN),  KC_QUOTE},
        {KC_LSFT,   KC_Z,       KC_X,       KC_C,           KC_V,           KC_B,       /* split */     KC_N,       KC_M,           KC_COMMA,   KC_DOT,     KC_SLASH,       KC_ENTER},
        {KC_LCTL,   KC_HOME,    KC_LALT,    KC_LGUI,        LOWER,          KC_SPC,     /* split */     KC_SPC,     RAISE,          KC_END,     KC_HOME,    KC_RSFT,        KC_RCTL}
    },

    [LAYER_LOWER] = {
        {KC_F1,    KC_F2,      KC_F3,      KC_F4,           KC_F5,          KC_F6,       /* split */     KC_F7,     KC_F8,          KC_F9,      KC_F10,     KC_F11,         KC_F12},
        {____,     LG_T(KC_1), LA_T(KC_2), LS_T(KC_3),      LC_T(KC_4),     KC_5,        /* split */     KC_6,      LC_T(KC_7),     LS_T(KC_8), LA_T(KC_9), LG_T(KC_0),     KC_MINUS},
        {____,     C_LEFT,     C_DOWN,     C_UP,            C_RIGHT,        ____,        /* split */     ____,      KC_LEFT,        KC_DOWN,    KC_UP,      KC_RIGHT,       ____},
        {____,     ____,       ____,       ____,            ____,           ____,        /* split */     ____,       ____,          ____,       ____,       ____,           ____}
    },

    [LAYER_RAISE] = {
        {____,      KC_BRKT_L,  KC_BRKT_R,  LS(KC_BRKT_L),  LS(KC_BRKT_R),  ____,       /* split */      ____,       LS(KC_BSLS),   KC_BSLS,    KC_EQ,      ____,           ____},
        {____,      S_1,        S_2,        S_3,            S_4,            S_5,        /* split */      S_6,        S_7,           S_8,        S_9,        S_0,            S_MINUS},
        {____,      ____,       ____,       ____,           ____,           ____,       /* split */      ____,       KC_LEFT,       KC_DOWN,    KC_UP,      KC_RIGHT,       ____},
        {____,      ____,       ____,       ____,           ____,           ____,       /* split */      ____,       ____,          ____,       ____,       ____,           ____}
    },

    [LAYER_FN] = {
        {____,      ____,       ____,       ____,           ____,           ____,       /* split */     ____,       ____,           ____,       ____,       ____,           ____},
        {____,      ____,       ____,       ____,           ____,           ____,       /* split */     ____,       ____,           ____,       ____,       ____,           ____},
        {____,      ____,       ____,       ____,           ____,           ____,       /* split */     ____,       ____,           ____,       ____,       ____,           ____},
        {____,      ____,       ____,       ____,           ____,           ____,       /* split */     ____,       ____,           ____,       ____,       ____,           ____}
    }
};

// private functions
static void keyboard_handle_layer_presses(void) {
    uint16_t key = KC_NONE;

    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            key = keymap[layer_state.current][row][col];

            if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
                switch (key & ENTRY_ARG_MASK) {
                    case LAYER_COM_MO: {
                        // A momentary layer switch is only active while the key is pressed, i.e. the state has to be re-evaluated every scan
                        layer_state.operation = LAYER_OPERATION_MO;
                        layer_state.current = key & KC_MASK;

                        // Don't process this entry on further operations
                        matrix_mark_key_as_handled(row, col);
                    } break;
                }
            }
        }
    }
}

static void keyboard_handle_taphold_presses(void) {
    uint16_t key = KC_NONE;

    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            // If this key is transparent, use the base layer instead
            key = keymap[layer_state.current][row][col];

            if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_TAPHOLD) {
                // Create a new taphold node from the pool
                ll_node_t* taphold_node = lla_alloc_tail(&tapholds.allocator);
                if (taphold_node != NULL) {
                    // Initialise the data
                    taphold_data_t* taphold = (taphold_data_t*)taphold_node->data;
                    taphold->hold_counter = 0;
                    taphold->layer = layer_state.current;
                    taphold->col = col;
                    taphold->row = row;
                }
                matrix_mark_key_as_handled(row, col);
            }
        }
    }
}

static void keyboard_handle_remaining_presses(void) {
    uint16_t key = KC_NONE;
    uint8_t kc_value = KC_NONE;

    // Now we have some certainty about the current layer, check for keypresses
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            // If this key is transparent, use the base layer instead
            key = keymap[layer_state.current][row][col];
            if (key == KC_TRANS) {
                key = keymap[layer_state.base][row][col];
            }
            kc_value = key & KC_MASK;

            switch (key & ENTRY_TYPE_MASK) {
                case ENTRY_TYPE_KC: {
                    // The key matrix entry arg can contain any combination of(left) modifiers
                    keyboard_hid_report_ref[0] |= (key >> 8);
                    keyboard_send_key(kc_value);

                    // this is a boot protocol keyboard, and it can't support more than 6 simultaneous keys.
                    // officially, we should be reporting an error through the hid protocol if there were more than 6, but let's skip
                    // that for now
                } break;
            }
        }
    }
}

static void keyboard_update_active_tapholds(void) {
    ll_node_t* current_node = tapholds.allocator.active_head;
    taphold_data_t* current_taphold = NULL;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;

        // Update the timer
        current_taphold->hold_counter += MATRIX_SCAN_INTERVAL_MS;
        if (current_taphold->hold_counter > TAP_HOLD_DELAY_MS) {
            current_taphold->hold_counter = TAP_HOLD_DELAY_MS;
        }

        // Is this key still being held? If the key was released or the layer changed, we need to stop tracking this taphold
        bool is_pressed = matrix_key_pressed(current_taphold->row, current_taphold->col, false);
        if (is_pressed) {
            // Since we've dealt with the key here, don't process it in further steps
            matrix_mark_key_as_handled(current_taphold->row, current_taphold->col);
        }

        // If the key was released before the hold delay timed out, we need to send the original key and cancel the taphold
        if (!is_pressed || (current_taphold->layer != layer_state.current)) {
            if (current_taphold->hold_counter < TAP_HOLD_DELAY_MS) {
                uint8_t kc_value = keymap[current_taphold->layer][current_taphold->row][current_taphold->col] & 0xff;
                keyboard_send_key(kc_value);
            }

            ll_node_t* next_current = current_node->next;
            lla_free(&tapholds.allocator, current_node);
            current_node = next_current;

            continue;
        }

        // If we got here, the key is still held. Check if it's active, and use it's modifiers if so
        if (current_taphold->hold_counter >= TAP_HOLD_DELAY_MS) {
            keyboard_hid_report_ref[0] |= (keymap[current_taphold->layer][current_taphold->row][current_taphold->col] >> 8) & 0xf;
        }

        // On to the next
        current_node = current_node->next;
    }
}

static bool keyboard_taphold_should_ignore_other_keys(void) {
    ll_node_t* current_node = tapholds.allocator.active_head;
    taphold_data_t* current_taphold = NULL;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;
        if (current_taphold->hold_counter < TAP_HOLD_DELAY_MS) {
            return true;
        }
        current_node = current_node->next;
        continue;
    }

    return false;
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

    // Init the taphold state
    lla_init(
        &tapholds.allocator,
        tapholds.data_array,
        tapholds.node_array,
        MAX_CONCURRENT_TAPHOLDS,
        sizeof(taphold_data_t)
    );
}

bool keyboard_send_key(uint8_t kc) {
    if (report_press_count >= 6) return false;

    // The key code value itself can be either a modifier or a key proper
    if ((kc >= KC_LCTL) && (kc <= KC_RGUI)) {
        // the bit position is encoded directly into the bottom nibble
        keyboard_hid_report_ref[0] |= (1 << (kc & 0xf));
    } else {
        // Avoid stuffing the same key in multiple times
        for (uint i = 2; i < 6; i++) {
            if (keyboard_hid_report_ref[i] == kc) return true;
        }
        keyboard_hid_report_ref[2 + report_press_count] = kc;
        ++report_press_count;
    }

    return true;
}

void keyboard_post_scan(void) {
    // Clear the report
    memset(keyboard_hid_report_ref, 0, 8);
    report_press_count = 0;

    // Process the keypresses in sequence, handling layer changes and more complex operations before simple presses
    keyboard_handle_layer_presses();

    keyboard_update_active_tapholds();
    keyboard_handle_taphold_presses();

    if (!keyboard_taphold_should_ignore_other_keys()) {
        keyboard_handle_remaining_presses();
    }

    // Once all the keys are processed, we can check if we need to do something with the layer state
    if (layer_state.operation == LAYER_OPERATION_MO) {
        layer_state.current = layer_state.base;
    }
}
