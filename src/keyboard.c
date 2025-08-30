/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "keyboard.h"
#include "taphold.h"
#include "doubletap.h"
#include "combo.h"
#include "matrix.h"

#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

// options/configuration
#define BOOTMAGIC_COL               (0)
#define BOOTMAGIC_ROW               (0)

// regular defines
#define LAYER_QWERTY            (0)
#define LAYER_LOWER             (1)
#define LAYER_RAISE             (2)
#define LAYER_FN                (3)
#define LAYER_MAX               (4)

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
} layer_state_t;

// statics
static layer_state_t layer_state = {
    .base = LAYER_QWERTY,
    .current = LAYER_QWERTY,
};

static uint8_t* keyboard_hid_report_ref = NULL;
static uint8_t report_press_count = 0;

static combo_t combos[NUM_COMBO_SLOTS] = {
    [0]  = COMBO2(KC_E,         KC_R,           LS(KC_9)),       // (
    [1]  = COMBO2(KC_U,         KC_I,           LS(KC_0)),       // )
    [2]  = COMBO2(KC_C,         KC_V,           KC_BRKT_L),      // [
    [3]  = COMBO2(KC_M,         KC_COMMA,       KC_BRKT_R),      // ]
    [4]  = COMBO2(KC_V,         KC_B,           LS(KC_BRKT_L)),  // {
    [5]  = COMBO2(KC_N,         KC_M,           LS(KC_BRKT_R)),  // }
    [6]  = COMBO2(KC_W,         KC_E,           KC_TAB),         // Tab
    [7]  = COMBO2(KC_I,         KC_O,           KC_TAB),         // Tab
    [8]  = COMBO2(KC_Q,         KC_W,           KC_CAPS),        // Caps Lock
    [9]  = COMBO_UNUSED,
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
        {____,      KC_BRKT_L,  KC_BRKT_R,  LS(KC_BRKT_L),  LS(KC_BRKT_R),  ____,       /* split */      ____,       LS(KC_BSLS),   KC_BSLS,    KC_EQ,      ____,           KC_DEL},
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

static void keyboard_on_key_release(uint row, uint col, keymap_entry_t key) {
    if (combo_on_key_release(row, col, key)) return;

    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
        if ((key & ENTRY_ARG8_MASK) == LAYER_COM_MO) {
            layer_state.current = layer_state.base;

            // If a momentary layer key is released, ignore active keypresses until they're released
            matrix_suppress_held_until_release();
        }
    }

    if (taphold_on_key_release(row, col, key)) return;
    if (double_tap_on_key_release(row, col, key)) return;
}

static void keyboard_on_key_press(uint row, uint col, keymap_entry_t key) {
    if (!tapholds_any_active()) {
        if (combo_on_key_press(row, col, key)) return;
    }

    // Handle layers
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
        if ((key & ENTRY_ARG8_MASK) == LAYER_COM_MO) {
            // A momentary layer switch is only active while the key is pressed
            layer_state.current = key & KC_MASK;

            // Don't process this entry on further operations
            matrix_mark_key_as_handled(row, col);

            return;
        }
    }

    if (taphold_on_key_press(row, col, key)) return;
    if (double_tap_on_key_press(row, col, key)) return;
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
        return;
    }

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

keymap_entry_t keyboard_resolve_key(uint row, uint col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return KC_NONE;

    keymap_entry_t key = keymap[layer_state.current][row][col];
    if (key == KC_TRANS) {
        key = keymap[layer_state.base][row][col];
    }

    return key;
}

keymap_entry_t keyboard_resolve_key_on_layer(uint row, uint col, uint layer) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS || layer >= LAYER_MAX) return KC_NONE;

    keymap_entry_t key = keymap[layer][row][col];
    if (key == KC_TRANS) {
        key = keymap[layer_state.base][row][col];
    }

    return key;
}


uint8_t keyboard_get_current_layer(void) {
    return layer_state.current;
}
