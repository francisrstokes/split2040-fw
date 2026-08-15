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

// defines
#define SPL_I2C_IDLE                    (0)
#define SPL_I2C_WRITE_IN_PROGRESS       (1U << 0U)
#define SPL_I2C_READ_IN_PROGRESS        (1U << 1U)
#define SPL_I2C_OPERATION_IN_PROGRESS   (SPL_I2C_WRITE_IN_PROGRESS | SPL_I2C_READ_IN_PROGRESS)

// typedefs
typedef struct split_i2c_target_t {
    uint32_t state;
    uint8_t address;
    bool read_started;
} split_i2c_target_t;

typedef struct split_i2c_controller_t {
    bool read_ready;
    uint32_t keys[MATRIX_ROWS];
} split_i2c_controller_t;

typedef enum split_uart_controller_state_t {
    split_uart_controller_state_wait_sync = 0,
    split_uart_controller_state_rx_keys,

    split_uart_controller_state_max
} split_uart_controller_state_t;

typedef struct split_uart_t {
    split_uart_controller_state_t state;
    uint8_t rx_buffer[4];
    uint32_t keys[MATRIX_ROWS];
    uint8_t row;
    uint8_t byte_offset;
} split_uart_t;

// public functions
void split_init(void);
void split_scan_complete(void);
void split_update(void);
uint32_t split_get_target_row(uint row);
