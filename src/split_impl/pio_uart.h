#pragma once

#include "../split.h"

// typedefs
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
split_impl_t* split_pio_uart_get_impl(void);
