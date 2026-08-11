/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

#if defined(SPLIT_ENABLE) && defined(SPLIT_TARGET)
#define SCAN_ONLY_MODE
#endif

// typedefs
typedef struct split_impl_t {
    void (*init)(void);
    void (*scan_complete)(void);
    void (*update)(void);
    uint32_t (*get_target_row)(uint row);
} split_impl_t;

// public functions
void split_init(void);
void split_scan_complete(void);
void split_update(void);
uint32_t split_get_target_row(uint row);
