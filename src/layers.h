/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// defines
#define LAYER_QWERTY            (0)
#define LAYER_LOWER             (1)
#define LAYER_RAISE             (2)
#define LAYER_FN                (3)
#define LAYER_SPLIT             (4)

#define LAYER_MAX               (5)

// typedefs
typedef struct layer_state_t {
    uint8_t base;
    uint8_t current;
} layer_state_t;

// public functions
bool layers_on_key_press(uint row, uint col, keymap_entry_t key);
bool layers_on_key_release(uint row, uint col, keymap_entry_t key);
bool layers_on_virtual_key(keymap_entry_t key);
uint8_t layers_get_current(void);
uint8_t layers_get_base(void);
void layers_set(uint8_t layer);
