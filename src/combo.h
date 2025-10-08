/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// typedefs
typedef struct rowcol_t {
    uint8_t row;
    uint8_t col;
} rowcol_t;

typedef enum combo_state_t {
    combo_state_invalid = 0,
    combo_state_inactive,
    combo_state_active,
    combo_state_cooldown,
    combo_state_wait_for_all_released,
    combo_state_single_held,
} combo_state_t;

typedef struct combo_t {
    combo_state_t state;
    uint8_t time_since_first_press;
    uint8_t keys_pressed_bitmask;
    keymap_entry_t keys[COMBO_KEYS_MAX];
    rowcol_t key_positions[COMBO_KEYS_MAX];
    keymap_entry_t key_out;
    uint8_t held_index;
} combo_t;

// helper macros for defining combos
#define COMBO(key0, key1, key2, key3, output_key)   {.state = combo_state_inactive, .keys = {key0, key1, key2, key3}, .key_out = output_key}
#define COMBO2(key0, key1, key_out)                 COMBO(key0, key1, KC_NONE, KC_NONE, key_out)
#define COMBO3(key0, key1, key2, key_out)           COMBO(key0, key1, key2, KC_NONE, key_out)
#define COMBO4(key0, key1, key2, key3, key_out)     COMBO(key0, key1, key2, key3, key_out)
#define COMBO_UNUSED                                {.state = combo_state_invalid}

// public functions
void combo_init(combo_t* combo_table);
bool combo_update(void);
bool combo_on_key_press(uint row, uint col, keymap_entry_t key);
bool combo_on_key_release(uint row, uint col, keymap_entry_t key);
