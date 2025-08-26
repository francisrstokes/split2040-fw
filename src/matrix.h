/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "hid.h"

// defines
#define MATRIX_SCAN_INTERVAL_MS     (10)
#define MATRIX_ROWS                 (4)
#define MATRIX_COLS                 (12)

// public functions
void matrix_init(void);
void matrix_scan(void);
bool matrix_key_pressed(uint32_t row, uint32_t col, bool also_when_handled);
bool matrix_key_pressed_this_scan(uint32_t row, uint32_t col);
bool matrix_key_released_this_scan(uint32_t row, uint32_t col);

void matrix_mark_key_as_handled(uint32_t row, uint32_t col);
void matrix_mark_key_as_unhandled(uint32_t row, uint32_t col);
const uint32_t* matrix_get_pressed_bitmap(void);
const uint32_t* matrix_get_handled_bitmap(void);
const uint matrix_get_col_gpio(uint col);
const uint matrix_get_row_gpio(uint row);
