/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "keyboard.h"
#include "taphold.h"
#include "matrix.h"

#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

// options/configuration
#define DOUBLE_TAP_DELAY_MS         (200)
#define NUM_COMBO_SLOTS             (16)
#define MAX_KEYS_PER_COMBO          (4)
#define COMBO_DELAY_MS              (50)

#define MAX_CONCURRENT_DOUBLE_TAPS  (8)

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

#define GRV_ESC                 TAP_HOLD(KC_ESC, KC_GRAVE, 0x00)

#define C_LEFT                  LC(KC_LEFT)
#define C_DOWN                  LC(KC_DOWN)
#define C_UP                    LC(KC_UP)
#define C_RIGHT                 LC(KC_RIGHT)

#define S_1                     LG_T(LS(KC_1))
#define S_2                     LA_T(LS(KC_2))
#define S_3                     LS_T(LS(KC_3))
#define S_4                     LC_T(LS(KC_4))
#define S_5                     LS(KC_5)
#define S_6                     LS(KC_6)
#define S_7                     LC_T(LS(KC_7))
#define S_8                     LS_T(LS(KC_8))
#define S_9                     LA_T(LS(KC_9))
#define S_0                     LG_T(LS(KC_0))
#define S_MINUS                 LS(KC_MINUS)

#define SPC_ENT                 DT(KC_SPC, KC_ENTER, 0x0)

// typedefs
typedef struct layer_state_t {
    uint8_t base;
    uint8_t current;
    uint8_t operation;
} layer_state_t;

typedef enum dt_state_t {
    dt_state_wait_first_release = 0,
    dt_state_wait_second_press,
    dt_state_single_tap,
    dt_state_double_tap
} dt_state_t;

typedef struct double_tap_data_t {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
    uint8_t time_since_first_tap;
    dt_state_t state;
} double_tap_data_t;

typedef struct double_tap_state_t {
    double_tap_data_t data_array[MAX_CONCURRENT_DOUBLE_TAPS];
    ll_node_t node_array[MAX_CONCURRENT_DOUBLE_TAPS];
    ll_allocator_t allocator;
} double_tap_state_t;

// statics
static layer_state_t layer_state = {
    .base = LAYER_QWERTY,
    .current = LAYER_QWERTY,
    .operation = LAYER_OPERATION_NONE
};

typedef struct rowcol_t {
    uint8_t row;
    uint8_t col;
} rowcol_t;

typedef enum combo_state_t {
    combo_state_invalid = 0,
    combo_state_inactive,
    combo_state_active,
    combo_state_cancelled
} combo_state_t;

typedef struct combo_t {
    combo_state_t state;
    uint8_t time_since_first_press;
    uint8_t keys_pressed_bitmask;
    keymap_entry_t keys[MAX_KEYS_PER_COMBO];
    rowcol_t key_positions[MAX_KEYS_PER_COMBO];
    keymap_entry_t key_out;
} combo_t;

#define COMBO(key0, key1, key2, key3, output_key)   {.state = combo_state_inactive, .keys = {key0, key1, key2, key3}, .key_out = output_key}
#define COMBO2(key0, key1, key_out)                 COMBO(key0, key1, KC_NONE, KC_NONE, key_out)
#define COMBO3(key0, key1, key2, key_out)           COMBO(key0, key1, key2, KC_NONE, key_out)
#define COMBO4(key0, key1, key2, key3, key_out)     COMBO(key0, key1, key2, key3, key_out)
#define COMBO_UNUSED                                {.state = combo_state_invalid}

static uint8_t* keyboard_hid_report_ref = NULL;
static uint8_t report_press_count = 0;
static double_tap_state_t double_taps = {0};
static combo_t combos[NUM_COMBO_SLOTS] = {
    [0]  = COMBO2(KC_E,         KC_R,           LS(KC_9)),       // (
    [1]  = COMBO2(KC_U,         KC_I,           LS(KC_0)),       // )
    [2]  = COMBO2(KC_C,         KC_V,           KC_BRKT_L),      // [
    [3]  = COMBO2(KC_M,         KC_COMMA,       KC_BRKT_R),      // ]
    [4]  = COMBO2(KC_V,         KC_B,           LS(KC_BRKT_L)),  // {
    [5]  = COMBO2(KC_N,         KC_M,           LS(KC_BRKT_R)),  // }
    [6]  = COMBO2(KC_W,         KC_E,           KC_TAB),         // Tab
    [7]  = COMBO2(KC_I,         KC_O,           KC_TAB),         // Tab
    [8]  = COMBO2(KC_O,         KC_P,           KC_CAPS),        // Caps Lock
    [9]  = COMBO2(KC_Q,         KC_W,           KC_CAPS),        // Caps Lock
    [10] = COMBO_UNUSED,
    [11] = COMBO_UNUSED,
    [12] = COMBO_UNUSED,
    [13] = COMBO_UNUSED,
    [14] = COMBO_UNUSED,
    [15] = COMBO_UNUSED,
};

static const keymap_entry_t keymap[NUM_LAYERS][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_QWERTY] = {
        {GRV_ESC,   KC_Q,       KC_W,       KC_E,           KC_R,           KC_T,       /* split */     KC_Y,       KC_U,           KC_I,       KC_O,       KC_P,           KC_BSPC},
        {KC_TAB,    LG_T(KC_A), LA_T(KC_S), LS_T(KC_D),     LC_T(KC_F),     KC_G,       /* split */     KC_H,       LC_T(KC_J),     LS_T(KC_K), LA_T(KC_L), LG_T(KC_SCLN),  KC_QUOTE},
        {KC_LSFT,   KC_Z,       KC_X,       KC_C,           KC_V,           KC_B,       /* split */     KC_N,       KC_M,           KC_COMMA,   KC_DOT,     KC_SLASH,       KC_ENTER},
        {KC_LCTL,   KC_HOME,    KC_LALT,    KC_LGUI,        LOWER,          SPC_ENT,    /* split */     KC_SPC,     RAISE,          KC_END,     KC_HOME,    KC_RSFT,        KC_RCTL}
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
        {KC_CAPS,   ____,       ____,       ____,           ____,           ____,       /* split */      ____,       ____,          ____,       ____,       ____,           ____}
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
    keymap_entry_t key = KC_NONE;

    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            key = keymap[layer_state.current][row][col];

            if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
                switch (key & ENTRY_ARG8_MASK) {
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

static void keyboard_handle_remaining_presses(void) {
    keymap_entry_t key = KC_NONE;

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

static void keyboard_update_active_double_taps(void) {
    ll_node_t* current_node = double_taps.allocator.active_head;
    double_tap_data_t* current_dt = NULL;
    keymap_entry_t key = KC_NONE;
    bool timer_expired = false;
    bool node_became_inactive = false;

    while (current_node != NULL) {
        current_dt = (double_tap_data_t*)current_node->data;

        // Update the timer
        current_dt->time_since_first_tap += MATRIX_SCAN_INTERVAL_MS;
        bool timer_expired = current_dt->time_since_first_tap >= DOUBLE_TAP_DELAY_MS;
        if (timer_expired) {
            current_dt->time_since_first_tap = DOUBLE_TAP_DELAY_MS;
        }

        bool key_is_pressed = matrix_key_pressed(current_dt->row, current_dt->col, true);

        // Handle the states where the outcome is not yet known
        if (current_dt->state == dt_state_wait_first_release) {
            if (timer_expired) {
                current_dt->state = dt_state_single_tap;
            } else if (!key_is_pressed) {
                current_dt->state = dt_state_wait_second_press;
            }
        } else if (current_dt->state == dt_state_wait_second_press) {
            if (timer_expired) {
                current_dt->state = dt_state_single_tap;
            } else if (key_is_pressed) {
                current_dt->state = dt_state_double_tap;
            }
        }

        // Handle taps and double taps (note: this accounts for state changes that occurred on this tick)
        key = keymap[current_dt->layer][current_dt->row][current_dt->col];
        if (current_dt->state == dt_state_single_tap) {
            keyboard_send_key(key);
            node_became_inactive = !key_is_pressed;
        } else if (current_dt->state == dt_state_double_tap) {
            keyboard_send_key((ENTRY_ARG4(key) << KEY_MODS_SHIFT ) | ENTRY_ARG8(key));
            node_became_inactive = !key_is_pressed;
        }

        if (node_became_inactive) {
            ll_node_t* next_current = current_node->next;
            lla_free(&double_taps.allocator, current_node);
            current_node = next_current;
            continue;
        }

        // On to the next
        current_node = current_node->next;
    }
}

static void keyboard_handle_double_tap_presses(void) {
    keymap_entry_t key = KC_NONE;

    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            // If this key is transparent, use the base layer instead
            key = keymap[layer_state.current][row][col];

            if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_DOUBLE_TAP) {
                // Create a new double tap node from the pool
                ll_node_t* dt_node = lla_alloc_tail(&double_taps.allocator);
                if (dt_node != NULL) {
                    // Initialise the data
                    double_tap_data_t* dt = (double_tap_data_t*)dt_node->data;
                    dt->time_since_first_tap = 0;
                    dt->layer = layer_state.current;
                    dt->col = col;
                    dt->row = row;
                    dt->state = dt_state_wait_first_release;
                }
                matrix_mark_key_as_handled(row, col);
            }
        }
    }
}

static bool keyboard_double_tap_should_ignore_other_keys(void) {
    ll_node_t* current_node = double_taps.allocator.active_head;
    double_tap_data_t* current_dt = NULL;

    while (current_node != NULL) {
        current_dt = (double_tap_data_t*)current_node->data;
        if (current_dt->time_since_first_tap < DOUBLE_TAP_DELAY_MS) {
            return true;
        }
        current_node = current_node->next;
        continue;
    }

    return false;
}

static void keyboard_handle_combo_presses(void) {
    keymap_entry_t key = KC_NONE;

    // Update timers, and check if cancelled combos have had all their keys released
    for (uint i = 0; i < NUM_COMBO_SLOTS; i++) {
        // The first invalid combo slot marks the end of defined combos
        if (combos[i].state == combo_state_invalid) break;

        // Update all the active timers, and mark any that have passed as inactive
        if (combos[i].state == combo_state_active) {
            combos[i].time_since_first_press += MATRIX_SCAN_INTERVAL_MS;
            if (combos[i].time_since_first_press >= COMBO_DELAY_MS) {
                combos[i].state = combo_state_cancelled;
            }
        } else if (combos[i].state == combo_state_cancelled) {
            // Have all the keys associated with this combo been released?
            bool all_released = true;
            for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                if (combos[i].keys[key_index] == KC_NONE) break;
                if (matrix_key_pressed(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col, true)) {
                    all_released = false;
                }
            }

            // When all keys have finally been released, go to a combo cooldown period
            if (all_released) {
                combos[i].state = combo_state_inactive;
                continue;
            } else {
                for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                    if (combos[i].keys[key_index] == KC_NONE) break;
                    matrix_mark_key_as_handled(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col);
                }
            }
        }
    }

    // Scan over the keys, noting those part of combos
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // We only care about keys that are pressed on this scan
            if (!matrix_key_pressed_this_scan(row, col)) continue;

            key = keymap[layer_state.current][row][col];

            for (uint i = 0; i < NUM_COMBO_SLOTS; i++) {
                if (combos[i].state == combo_state_invalid) break;
                if (combos[i].state == combo_state_cancelled) continue;

                for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                    // A none key marks the end of the keys involved in this combo
                    if (combos[i].keys[key_index] == KC_NONE) continue;

                    if (key == combos[i].keys[key_index]) {
                        if (combos[i].state == combo_state_inactive) {
                            combos[i].state = combo_state_active;
                            // Set all the key positions to 0xff, since 0x00 will always be a valid column and row and could cause misfires
                            memset(combos[i].key_positions, 0xff, sizeof(combos[i].key_positions));
                            combos[i].time_since_first_press = 0;
                        }

                        combos[i].keys_pressed_bitmask |= (1 << key_index);
                        combos[i].key_positions[key_index].row = row;
                        combos[i].key_positions[key_index].col = col;
                        matrix_mark_key_as_handled(row, col);
                    }
                }
            }
        }
    }

    // Loop through the active combos, and emit any which were fulfilled
    for (uint i = 0; i < NUM_COMBO_SLOTS; i++) {
        if (combos[i].state == combo_state_invalid) break;
        if (combos[i].state != combo_state_active) continue;

        // First of all, were any of the keys that we had marked as pressed released this scan?
        bool cancelled = false;
        for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
            if (combos[i].keys[key_index] == KC_NONE) break;
            if (matrix_key_released_this_scan(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col)) {
                cancelled = true;
            }
        }

        if (cancelled) {
            combos[i].state = combo_state_cancelled;
            continue;
        }

        bool all_keys_pressed = true;
        for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
            if (combos[i].keys[key_index] == KC_NONE) break;

            // Also check handled keys, since any pressed on this scan would have been marked
            if (!matrix_key_pressed(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col, true)) {
                all_keys_pressed = false;
                break;
            }
        }

        // Check if all were pressed, and send combo key
        if (all_keys_pressed) {
            keyboard_send_key(combos[i].key_out);
            combos[i].state = combo_state_cancelled;

            // Mark the keys involved as handled
            for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                if (combos[i].keys[key_index] == KC_NONE) break;
                matrix_mark_key_as_handled(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col);
            }
        }
    }
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
    lla_init(
        &double_taps.allocator,
        double_taps.data_array,
        double_taps.node_array,
        MAX_CONCURRENT_DOUBLE_TAPS,
        sizeof(double_tap_data_t)
    );
}

bool keyboard_send_key(keymap_entry_t key) {
    if (report_press_count >= 6) return false;

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

void keyboard_post_scan(void) {
    // Clear the report
    memset(keyboard_hid_report_ref, 0, 8);
    report_press_count = 0;

    // Quick and dirty reset to bootloader, should move this to a proper key handler
    if (matrix_key_pressed(0, 0, true) && matrix_key_pressed(1, 1, true) && matrix_key_pressed(2, 2, true)) {
        reset_usb_boot(0, 0);
    }

    // Handle combos before layer change operations to allow for the layer changing keys themselves to be used for combos
    keyboard_handle_combo_presses();

    // Process the keypresses in sequence, handling layer changes and more complex operations before simple presses
    keyboard_handle_layer_presses();

    bool ignore_from_taphold = taphold_update();

    keyboard_update_active_double_taps();
    keyboard_handle_double_tap_presses();

    if (!(ignore_from_taphold || keyboard_double_tap_should_ignore_other_keys())) {
        keyboard_handle_remaining_presses();
    }

    // Once all the keys are processed, we can check if we need to do something with the layer state
    if (layer_state.operation == LAYER_OPERATION_MO) {
        layer_state.current = layer_state.base;
    }
}

keymap_entry_t keyboard_resolve_key(uint row, uint col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return KC_NONE;

    keymap_entry_t key = keymap[layer_state.current][row][col];
    if (key == KC_TRANS) {
        key = keymap[layer_state.base][row][col];
    }

    return key;
}

uint8_t keyboard_get_current_layer(void) {
    return layer_state.current;
}
