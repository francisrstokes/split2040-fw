/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrix.h"
#include "keyboard.h"

#include <string.h>

#include "pico/stdlib.h"

// Define
#define MATRIX_SETTLE_ITERATIONS (50)

// statics
static uint32_t prev_pressed_bitmap[MATRIX_ROWS] = {0};
static uint32_t pressed_bitmap[MATRIX_ROWS] = {0};
static uint32_t handled_bitmap[MATRIX_ROWS] = {0};
static uint32_t pressed_this_scan_bitmap[MATRIX_ROWS] = {0};
static uint32_t released_this_scan_bitmap[MATRIX_ROWS] = {0};
static uint32_t suppressed_until_release[MATRIX_ROWS] = {0};

static uint matrix_cols[MATRIX_COLS] = { 21, 20, 19, 18, 17, 16,        11, 12, 9, 10, 7, 8 };
static uint matrix_rows[MATRIX_ROWS] = { 2, 3, 4, 5 };

// private functions
static inline void matrix_settle_delay(void) {
    for (uint i = 0; i < MATRIX_SETTLE_ITERATIONS; i++) {
        asm volatile("nop\n");
    }
}

// public functions
void matrix_init(void) {
    // Scan asserts a high on a column and reads back the rows
    for (uint i = 0; i < MATRIX_COLS; i++) {
        gpio_init(matrix_cols[i]);
        gpio_set_dir(matrix_cols[i], GPIO_OUT);
        gpio_put(matrix_cols[i], false);
    }

    // Rows are pulled down
    for (uint i = 0; i < MATRIX_ROWS; i++) {
        gpio_init(matrix_rows[i]);
        gpio_set_dir(matrix_rows[i], GPIO_IN);
        gpio_pull_down(matrix_rows[i]);
    }
}

void matrix_scan(void) {
    // Copy the last scan to the previous
    memcpy(prev_pressed_bitmap, pressed_bitmap, sizeof(pressed_bitmap));

    // Clear the pressed key positions
    memset(pressed_bitmap, 0, sizeof(pressed_bitmap));
    memset(handled_bitmap, 0, sizeof(handled_bitmap));
    memset(pressed_this_scan_bitmap, 0, sizeof(pressed_this_scan_bitmap));
    memset(released_this_scan_bitmap, 0, sizeof(released_this_scan_bitmap));

    // Scan each column in turn, reading back the rows
    for (uint col = 0; col < MATRIX_COLS; col++) {
        // Assert the column
        gpio_put(matrix_cols[col], true);
        matrix_settle_delay();

        // Scan the rows
        for (uint row = 0; row < MATRIX_ROWS; row++) {
            if (gpio_get(matrix_rows[row])) {
                pressed_bitmap[row] |= (1 << col);
            }
        }

        // Deassert the column
        gpio_put(matrix_cols[col], false);
        matrix_settle_delay();
    }

    // Compute the deltas
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        pressed_this_scan_bitmap[row] = ~prev_pressed_bitmap[row] & pressed_bitmap[row];
        released_this_scan_bitmap[row] = prev_pressed_bitmap[row] & ~pressed_bitmap[row];
        suppressed_until_release[row] &= ~released_this_scan_bitmap[row];
    }

    // Once the scan is complete, hand off to the keyboard to process the key presses
    keyboard_post_scan();
}

bool matrix_key_pressed(uint32_t row, uint32_t col, bool also_when_handled) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;

    // Start with all currently pressed keys
    uint32_t row_data = pressed_bitmap[row];

    // Filter out the handled keys if configured
    if (!also_when_handled) {
        row_data &= ~handled_bitmap[row];
    }

    // Always filter out suppressed keys
    row_data &= ~suppressed_until_release[row];

    return ((row_data >> col) & 1) == 1;
}

bool matrix_key_pressed_this_scan(uint32_t row, uint32_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;
    return ((pressed_this_scan_bitmap[row] >> col) & 1) == 1;
}

bool matrix_key_released_this_scan(uint32_t row, uint32_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return false;
    return ((released_this_scan_bitmap[row] >> col) & 1) == 1;
}

void matrix_suppress_held_until_release(void) {
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        suppressed_until_release[row] |= pressed_bitmap[row];
    }
}

void matrix_suppress_key_until_release(uint32_t row, uint32_t col) {
    // Only actually supress the key when it is already held
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return;
    suppressed_until_release[row] |= (pressed_bitmap[row] & (1 << col));
}

void matrix_mark_key_as_handled(uint32_t row, uint32_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return;
    handled_bitmap[row] |= 1 << col;
}

void matrix_mark_key_as_unhandled(uint32_t row, uint32_t col) {
    if (row >= MATRIX_ROWS || col >= MATRIX_COLS) return;
    handled_bitmap[row] &= ~(1 << col);
}

const uint32_t* matrix_get_pressed_bitmap(void) {
    return (const uint32_t*)pressed_bitmap;
}

const uint32_t* matrix_get_handled_bitmap(void) {
    return (const uint32_t*)handled_bitmap;
}

const uint32_t* matrix_get_released_this_scan_bitmap(void) {
    return (const uint32_t*)released_this_scan_bitmap;
}

const uint32_t* matrix_get_pressed_this_scan_bitmap(void) {
    return (const uint32_t*)pressed_this_scan_bitmap;
}

const uint matrix_get_col_gpio(uint col) {
    return matrix_cols[col];
}

const uint matrix_get_row_gpio(uint row) {
    return matrix_rows[row];
}
