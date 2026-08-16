/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "ll_alloc.h"
#include "keyboard.h"
#include "keyboard_system.h"

// typedefs
typedef struct taphold_data_t {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
    uint16_t hold_counter;
} taphold_data_t;

typedef struct taphold_state_t {
    taphold_data_t data_array[TAP_HOLD_MAX];
    ll_node_t node_array[TAP_HOLD_MAX];
    ll_allocator_t allocator;
} taphold_state_t;

// public functions
const keyboard_system_t* taphold_system(void);
bool taphold_any_active(void);
