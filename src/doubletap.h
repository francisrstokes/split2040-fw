/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "ll_alloc.h"
#include "keyboard.h"

// defines
#define DOUBLE_TAP_DELAY_MS         (200)
#define MAX_CONCURRENT_DOUBLE_TAPS  (8)

// typedefs
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

// public functions
void double_tap_init(void);
bool double_tap_update(void);
bool double_tap_on_key_release(uint row, uint col, keymap_entry_t key);
bool double_tap_on_key_press(uint row, uint col, keymap_entry_t key);
