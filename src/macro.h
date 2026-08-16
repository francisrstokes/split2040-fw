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
typedef enum macro_type_t {
    macro_type_unused = 0,
    macro_type_send_string,
} macro_type_t;

typedef struct macro_send_string_t {
    const char* buffer;
    uint32_t length;
    uint32_t index;
    bool was_release;
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
const keyboard_system_t* macro_system(void);
bool macro_any_active(void);
