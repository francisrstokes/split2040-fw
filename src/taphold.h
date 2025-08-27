/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "ll_alloc.h"

// defines
#define TAP_HOLD_DELAY_MS           (200)
#define MAX_CONCURRENT_TAPHOLDS     (8)

// typedefs
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

// public functions
void taphold_init(void);
bool taphold_update(void);