/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"
#include "keyboard_system.h"

// typedefs
typedef struct layer_state_t {
    uint8_t base;
    uint8_t current;
} layer_state_t;

// public functions
const keyboard_system_t* layers_system(void);
uint8_t layers_get_current(void);
uint8_t layers_get_base(void);
void layers_set(uint8_t layer);

// weak functions, to be implemented by the specific keyboard
void layer_post_set(uint8_t layer);
