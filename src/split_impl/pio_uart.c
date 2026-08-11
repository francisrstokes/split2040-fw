#include "pio_uart.h"
#include "../pio_uart.h"
#include "../matrix.h"

#include <string.h>

#ifdef SPLIT_PIO_UART

// forwards
static void split_pio_uart_init(void);
static void split_pio_uart_update(void);
static uint32_t split_pio_uart_get_target_row(uint row);
static void split_pio_uart_scan_complete(void);

// statics
static split_impl_t split_pio_uart_impl = {
    .init = split_pio_uart_init,
    .scan_complete = split_pio_uart_scan_complete,
    .update = split_pio_uart_update,
    .get_target_row = split_pio_uart_get_target_row,
};

static const uint8_t sync_seq[] = {'k', 'e', 'e', 'b'};
static pio_uart_t split_pio_uart = {0};
static split_uart_t split_uart = {0};

// irqs
static void split_uart_rx_irq(uint8_t byte) {
#if defined(SPLIT_CONTROLLER)
    split_uart.rx_buffer[split_uart.byte_offset] = byte;
    split_uart.byte_offset =  (split_uart.byte_offset + 1) % sizeof(split_uart.rx_buffer);

    switch (split_uart.state) {
        case split_uart_controller_state_wait_sync: {
            // We're waiting for the sync sequence. Check if its there
            const bool synced = (
                   split_uart.rx_buffer[(split_uart.byte_offset + 0) % sizeof(split_uart.rx_buffer)] == sync_seq[0]
                && split_uart.rx_buffer[(split_uart.byte_offset + 1) % sizeof(split_uart.rx_buffer)] == sync_seq[1]
                && split_uart.rx_buffer[(split_uart.byte_offset + 2) % sizeof(split_uart.rx_buffer)] == sync_seq[2]
                && split_uart.rx_buffer[(split_uart.byte_offset + 3) % sizeof(split_uart.rx_buffer)] == sync_seq[3]
            );

            if (synced) {
                split_uart.state = split_uart_controller_state_rx_keys;
                // Reset the circular buffer so that when each row is complete, it can be directly memcpy'd to the keys buffer
                split_uart.byte_offset = 0;
                split_uart.row = 0;
            }
        } break;

        case split_uart_controller_state_rx_keys: {
            // Have we read a full row?
            if (split_uart.byte_offset == 0) {
                memcpy(&split_uart.keys[split_uart.row], split_uart.rx_buffer, 4);

                if (++split_uart.row >= MATRIX_ROWS) {
                    // We have the latest state, go back to waiting
                    split_uart.state = split_uart_controller_state_wait_sync;
                }
            }
        } break;
    }
#endif
}

// private functions
static void split_pio_uart_init(void) {
#ifdef SPLIT_CONTROLLER
#define SPLIT_UART_TX SPLIT_UART_CONTROLLER_TX
#define SPLIT_UART_RX SPLIT_UART_CONTROLLER_RX
#else
#define SPLIT_UART_TX SPLIT_UART_TARGET_TX
#define SPLIT_UART_RX SPLIT_UART_TARGET_RX
#endif

    pio_uart_init(&split_pio_uart, SPLIT_PIO_UART, SPLIT_UART_TX, SPLIT_UART_RX, SPLIT_UART_BAUDRATE, split_uart_rx_irq);
}

static void split_pio_uart_update(void) {
    // (nothing)
}

static uint32_t split_pio_uart_get_target_row(uint row) {
    if (row >= MATRIX_ROWS) {
        return 0;
    }
    return split_uart.keys[row];
}

static void split_pio_uart_scan_complete(void) {
    pio_uart_write_blocking(&split_pio_uart, sync_seq, sizeof(sync_seq));
    pio_uart_write_blocking(&split_pio_uart, (const uint8_t*)matrix_get_pressed_bitmap(), sizeof(split_uart.keys));
}

// public functions
split_impl_t* split_pio_uart_get_impl(void) {
    return &split_pio_uart_impl;
}

#endif
