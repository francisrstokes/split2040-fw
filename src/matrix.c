/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrix.h"
#include "keyboard.h"

#include <string.h>

#include "pico/stdlib.h"

// statics
static uint32_t pressed_bitmap[MATRIX_ROWS] = {0};
static uint32_t handled_bitmap[MATRIX_ROWS] = {0};

// private functions
static uint matrix_cols[MATRIX_COLS] = { 5, 4, 3, 2, 1, 0, 20, 21, 22, 26, 27, 28 };
static uint matrix_rows[MATRIX_ROWS] = { 19, 18, 17, 16 };

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
    // Clear the pressed key positions
    memset(pressed_bitmap, 0, sizeof(pressed_bitmap));
    memset(handled_bitmap, 0, sizeof(handled_bitmap));

    // Scan each column in turn, reading back the rows
    for (uint col = 0; col < MATRIX_COLS; col++) {
        // Assert the column
        gpio_put(matrix_cols[col], true);

        // Scan the rows
        for (uint row = 0; row < MATRIX_ROWS; row++) {
            if (gpio_get(matrix_rows[row])) {
                pressed_bitmap[row] |= (1 << col);
            }
        }

        // Deassert the column
        gpio_put(matrix_cols[col], false);

        // Wait some time to avoid asserting multiple columns at the same time
        for (uint i = 0; i < 25; i++) {
            asm volatile("nop\n");
        }
    }

    // Once the scan is complete, hand off to the keyboard to process the key presses
    keyboard_post_scan();
}

bool matrix_key_pressed(uint32_t row, uint32_t col, bool also_when_handled) {
    uint32_t row_data = pressed_bitmap[row];
    if (!also_when_handled) {
        row_data &= ~handled_bitmap[row];
    }
    return ((row_data >> col) & 1) == 1;
}

void matrix_mark_key_as_handled(uint32_t row, uint32_t col) {
    handled_bitmap[row] |= 1 << col;
}

const uint32_t* matrix_get_pressed_bitmap(void) {
    return (const uint32_t*)pressed_bitmap;
}

const uint32_t* matrix_get_handled_bitmap(void) {
    return (const uint32_t*)handled_bitmap;
}

const uint matrix_get_col_gpio(uint col) {
    return matrix_cols[col];
}

const uint matrix_get_row_gpio(uint row) {
    return matrix_rows[row];
}
