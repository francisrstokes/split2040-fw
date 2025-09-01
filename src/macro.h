/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// defines
#define NUM_MACRO_SLOTS (8)

// typedefs
typedef enum macro_type_t {
    macro_type_unused,
    macro_type_send_string,
} macro_type_t;

typedef struct macro_send_string_t {
    const char* buffer;
    const uint32_t length;
    uint32_t index;
} macro_send_string_t;

typedef struct macro_t {
    macro_type_t type;
    bool active;
    union {
        macro_send_string_t send_string;
    };
} macro_t;

// helper macros for defining macros
#define SEND_STRING(char_buf, len)     { .type = macro_type_send_string, .active = false, .send_string = { .buffer = char_buf, .length = len }}
#define MACRO_UNUSED                   { .type = macro_type_unused, .active = false }

// public functions
void macro_init(macro_t* macro_table);
bool macro_on_key_press(uint row, uint col, keymap_entry_t key);
bool macro_on_key_release(uint row, uint col, keymap_entry_t key);
bool macro_on_virtual_key(keymap_entry_t key);
bool macro_update(void);
bool macro_any_active(void);
