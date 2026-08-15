#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "hardware/pio.h"

// typedefs
typedef void (*pio_uart_rx_irq_t)(uint8_t byte);

typedef struct pio_uart_t {
    PIO  pio;
    uint sm_tx;
    uint sm_rx;
} pio_uart_t;

// public functions
bool pio_uart_init(pio_uart_t *u, PIO pio, uint pin_tx, uint pin_rx, uint baud, pio_uart_rx_irq_t rx_cb);
void pio_uart_write_blocking(pio_uart_t *u, const uint8_t *data, uint len);
