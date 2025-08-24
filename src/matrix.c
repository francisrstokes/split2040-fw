/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrix.h"
#include "ll_alloc.h"

#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

// defines

// options/configuration
#define TAP_HOLD_DELAY_MS           (150)
#define MAX_CONCURRENT_TAPHOLDS     (8)
#define MAX_PRESSED_POSITIONS       (32)

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
typedef struct pressed_pos_t {
    uint8_t col;
    uint8_t row;
} pressed_pos_t;

typedef struct layer_state_t {
    uint8_t base;
    uint8_t current;
    uint8_t operation;
} layer_state_t;

typedef struct taphold_data_t {
    pressed_pos_t pos;
    uint8_t layer;
    uint8_t hold_counter;
    bool force_tap;
} taphold_data_t;

typedef struct taphold_state_t {
    taphold_data_t data_array[MAX_CONCURRENT_TAPHOLDS];
    ll_node_t node_array[MAX_CONCURRENT_TAPHOLDS];
    ll_allocator_t allocator;
} taphold_state_t;

// statics
static uint matrix_cols[MATRIX_COLS] = { 5, 4, 3, 2, 1, 0, 20, 21, 22, 26, 27, 28 };
static uint matrix_rows[MATRIX_ROWS] = { 19, 18, 17, 16 };

static layer_state_t layer_state = {
    .base = LAYER_QWERTY,
    .current = LAYER_QWERTY,
    .operation = LAYER_OPERATION_NONE
};

static uint8_t report_press_count = 0;
static uint8_t press_count = 0;
static uint8_t handled_count = 0;
static pressed_pos_t pressed_positions[MAX_PRESSED_POSITIONS] = {0};

static taphold_state_t tapholds = {0};

static uint16_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_QWERTY] = {
        {KC_ESC,    KC_Q,       KC_W,       KC_E,       KC_R,       KC_T,       /* split */     KC_Y,       KC_U,       KC_I,       KC_O,       KC_P,           KC_BSPC},
        {KC_TAB,    LG_T(KC_A), LA_T(KC_S), LS_T(KC_D), LC_T(KC_F), KC_G,       /* split */     KC_H,       LC_T(KC_J), LS_T(KC_K), LA_T(KC_L), LG_T(KC_SCLN),  KC_QUOTE},
        {KC_LSFT,   KC_Z,       KC_X,       KC_C,       KC_V,       KC_B,       /* split */     KC_N,       KC_M,       KC_COMMA,   KC_DOT,     KC_SLASH,       KC_ENTER},
        {KC_LCTL,   KC_HOME,    KC_LALT,    KC_LGUI,    LOWER,      KC_SPC,     /* split */     KC_SPC,     RAISE,      KC_END,     KC_HOME,    KC_RSFT,        KC_RCTL}
    },

    [LAYER_LOWER] = {
        {KC_F1,    KC_F2,      KC_F3,      KC_F4,      KC_F5,      KC_F6,       /* split */     KC_F7,     KC_F8,       KC_F9,      KC_F10,     KC_F11,         KC_F12},
        {____,     LG_T(KC_1), LA_T(KC_2), LS_T(KC_3), LC_T(KC_4), KC_5,        /* split */     KC_6,      LC_T(KC_7),  LS_T(KC_8), LA_T(KC_9), LG_T(KC_0),     KC_MINUS},
        {____,     C_LEFT,     C_DOWN,     C_UP,       C_RIGHT,    ____,        /* split */     ____,      KC_LEFT,     KC_DOWN,    KC_UP,      KC_RIGHT,       ____},
        {____,     ____,       ____,       ____,       ____,       ____,        /* split */     ____,       ____,       ____,       ____,       ____,           ____}
    },

    [LAYER_RAISE] = {
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,           ____},
        {____,      S_1,        S_2,        S_3,        S_4,        S_5,        /* split */     S_6,        S_7,        S_8,        S_9,        S_0,            S_MINUS},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       KC_LEFT,    KC_DOWN,    KC_UP,      KC_RIGHT,       ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,           ____}
    },

    [LAYER_FN] = {
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,           ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,           ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,           ____},
        {____,      ____,       ____,       ____,       ____,       ____,       /* split */     ____,       ____,       ____,       ____,       ____,           ____}
    }
};

// private functions
static void matrix_bootmagic(void) {
    gpio_put(matrix_cols[0], true);
    sleep_ms(1);
    if (gpio_get(matrix_rows[0])) {
        reset_usb_boot(0, 0);
    }
    gpio_put(matrix_cols[0], false);
}

static void matrix_add_key_to_report(uint8_t* keyboard_hid_report, uint8_t kc) {
    if (report_press_count < 6) {
        // The key code value itself can be either a modifier or a key proper
        if ((kc >= KC_LCTL) && (kc <= KC_RGUI)) {
            // the bit position is encoded directly into the bottom nibble
            keyboard_hid_report[0] |= (1 << (kc & 0xf));
        } else {
            // Avoid stuffing the same key in multiple times
            for (uint i = 2; i < 6; i++) {
                if (keyboard_hid_report[i] == kc) return;
            }
            keyboard_hid_report[2 + report_press_count] = kc;
            ++report_press_count;
        }
    }
}

static int matrix_find_index_of_pressed_key(uint8_t col, uint8_t row) {
    for (int i = 0; i < press_count; i++) {
        if ((pressed_positions[i].col == col) && (pressed_positions[i].row == row)) {
            return i;
        }
    }

    return -1;
}

static void matrix_update_active_tapholds(uint8_t* keyboard_hid_report) {
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
        int press_index = matrix_find_index_of_pressed_key(current_taphold->pos.col, current_taphold->pos.row);
        if (press_index != -1) {
            // Since we've dealt with the key here, don't process it in further steps
            pressed_positions[press_index].col = PRESS_PROCESSED;
            ++handled_count;
        }

        // If the key was released before the hold delay timed out, we need to send the original key and cancel the taphold
        if ((press_index == -1) || (current_taphold->layer != layer_state.current)) {
            if (current_taphold->hold_counter < TAP_HOLD_DELAY_MS && !current_taphold->force_tap) {
                uint8_t kc_value = keymap[current_taphold->layer][current_taphold->pos.row][current_taphold->pos.col] & 0xff;
                matrix_add_key_to_report(keyboard_hid_report, kc_value);
            }

            ll_node_t* next_current = current_node->next;
            lla_free(&tapholds.allocator, current_node);
            current_node = next_current;

            continue;
        }

        // If we got here, the key is still held. Check if it's active, and use it's modifiers if so
        if (current_taphold->force_tap) {
            uint8_t kc_value = keymap[current_taphold->layer][current_taphold->pos.row][current_taphold->pos.col] & 0xff;
            matrix_add_key_to_report(keyboard_hid_report, kc_value);
        } else if (current_taphold->hold_counter >= TAP_HOLD_DELAY_MS) {
            keyboard_hid_report[0] |= (keymap[current_taphold->layer][current_taphold->pos.row][current_taphold->pos.col] >> 8) & 0xf;
        }

        // On to the next
        current_node = current_node->next;
    }
}

static void matrix_check_for_taphold_interruptions(uint8_t* keyboard_hid_report) {
    if (press_count != handled_count) {
        ll_node_t* current_node = tapholds.allocator.active_head;
        taphold_data_t* current_taphold = NULL;

        while (current_node != NULL) {
            current_taphold = (taphold_data_t*)current_node->data;
            if (!current_taphold->force_tap && current_taphold->hold_counter < TAP_HOLD_DELAY_MS) {
                uint8_t kc_value = keymap[current_taphold->layer][current_taphold->pos.row][current_taphold->pos.col] & 0xff;
                matrix_add_key_to_report(keyboard_hid_report, kc_value);

                // Until released, this taphold only emits its tap kc, regardless of hold counter
                current_taphold->force_tap = true;
            } else {
                current_node = current_node->next;
            }
            continue;
        }
    }
}

static void matrix_handled_pressed_keys(uint8_t* keyboard_hid_report) {
    uint16_t key = KC_NONE;
    uint8_t kc_value = KC_NONE;

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

                        ++handled_count;
                    } break;
                }
            } break;

            // Skip everything else, since we can't know which layer to reference in the keymap
        }
    }

    // Before processing keys, update the active tapholds
    matrix_update_active_tapholds(keyboard_hid_report);

    // Before handling leftover regular keypresses, if there are any tapholds that were not yet in the holding state,
    // but there are remaining keypresses, interrupt them
    matrix_check_for_taphold_interruptions(keyboard_hid_report);

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
            case ENTRY_TYPE_TAPHOLD: {
                // Create a new taphold node from the pool
                ll_node_t* taphold_node = lla_alloc_tail(&tapholds.allocator);
                if (taphold_node != NULL) {
                    // Initialise the data
                    taphold_data_t* taphold = (taphold_data_t*)taphold_node->data;
                    taphold->force_tap = false;
                    taphold->hold_counter = 0;
                    taphold->layer = layer_state.current;
                    taphold->pos = pressed_positions[i];
                }
            } break;

            case ENTRY_TYPE_KC: {
                // The key matrix entry arg can contain any combination of(left) modifiers
                keyboard_hid_report[0] |= (key >> 8);
                matrix_add_key_to_report(keyboard_hid_report, kc_value);

                // because this is a boot protocol keyboard, it can't support more than 6 simultaneous keys, so early exit.
                // officially, we should be reporting an error through the hid protocol if there were more than 6, but let's skip
                // that for now
                if (report_press_count >= 6) {
                    i = press_count;
                    break;
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

    // Reset to the bootrom if the escape key is held during boot
    matrix_bootmagic();

    // Init the taphold state
    lla_init(
        &tapholds.allocator,
        tapholds.data_array,
        tapholds.node_array,
        MAX_CONCURRENT_TAPHOLDS,
        sizeof(taphold_data_t)
    );
}

void matrix_scan(uint8_t* keyboard_hid_report) {
    // Clear the report
    memset(keyboard_hid_report, 0, 8);

    // Clear the pressed key positions
    memset(pressed_positions, 0, sizeof(pressed_positions));
    press_count = 0;
    handled_count = 0;
    report_press_count = 0;

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
