#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#include <string.h>

#include "split.h"
#include "matrix.h"
#include "pio_uart.h"

// statics
#ifdef SPLIT_PIO_UART
static const uint8_t sync_seq[] = {'k', 'e', 'e', 'b'};
static pio_uart_t split_pio_uart = {0};
static split_uart_t split_uart = {0};
#endif

#ifdef SPLIT_I2C
static split_i2c_target_t split_i2c_target = {
    .state = SPL_I2C_IDLE,
    .address = 0,
    .read_started = false
};

static split_i2c_controller_t split_i2c_controller = {
    .read_ready = false,
    .keys = {0}
};
#endif

// I2C IRQs
#ifdef SPLIT_I2C
// I2C target IRQ
static void split_i2c_target_irq(void) {
#ifdef SPLIT_I2C
    i2c_hw_t* hw = SPLIT_I2C->hw;
    uint32_t intr_stat = hw->intr_stat;

    if (intr_stat == 0) {
        return;
    }

    bool do_reset_state = false;

    // Transmit abort?
    if (intr_stat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        hw->clr_tx_abrt;
        do_reset_state = true;
    }

    // Start detect?
    if (intr_stat & I2C_IC_INTR_STAT_R_START_DET_BITS) {
        hw->clr_start_det;
        do_reset_state = true;
    }

    // Restart detect?
    if (intr_stat & I2C_IC_INTR_STAT_R_RESTART_DET_BITS) {
        hw->clr_restart_det;
        do_reset_state = true;
    }

    // Finished with read or write?
    if (do_reset_state && (split_i2c_target.state & SPL_I2C_OPERATION_IN_PROGRESS)) {
        split_i2c_target.state = SPL_I2C_IDLE;
        split_i2c_target.read_started = false;
        split_i2c_target.address = 0;
    }

    // Read operation
    if (intr_stat & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
        hw->clr_rd_req;

        split_i2c_target.state = SPL_I2C_READ_IN_PROGRESS;

        // Clear the interrupt towards the controller (if enabled)
        if (!split_i2c_target.read_started) {
            split_i2c_target.read_started = true;
#ifdef SPLIT_INTERRUPT
            gpio_put(SPLIT_INTERRUPT, false);
#endif
        }

        const uint32_t* keys_bitmap = matrix_get_pressed_bitmap();

        // Try to pack the TX fifo full with as much data as possible. This reduces the time spent clock stretching to a minimum.
        // This is done in a do-while loop instead of a while so that, if the controller is somehow out of sync and asking for more
        // data than we have, we can still give them something (a zero).
        do {
            const uint8_t row = split_i2c_target.address / MATRIX_ROWS;
            const uint8_t shift = split_i2c_target.address & 0x3;
            const uint8_t tx_value = (row < MATRIX_ROWS)
                ? (keys_bitmap[row] >> shift) & 0xff
                : 0;

            // Write directly into the transmit fifo register
            SPLIT_I2C->hw->data_cmd = tx_value;
            ++split_i2c_target.address;
        } while (split_i2c_target.address < (sizeof(uint32_t) * MATRIX_ROWS) && !(SPLIT_I2C->hw->status & I2C_IC_STATUS_TFNF_BITS));
    }

    // Write operation
    if (intr_stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        split_i2c_target.state = SPL_I2C_WRITE_IN_PROGRESS;

        if (!split_i2c_target.read_started) {
            split_i2c_target.read_started = true;

            uint8_t command = i2c_read_byte_raw(SPLIT_I2C);
            // Do something with command...
        }
    }
#endif
}

// I2C controller IRQ
static void split_target_ready_for_read_irq(void) {
#ifdef SPLIT_I2C
    gpio_acknowledge_irq(SPLIT_INTERRUPT, GPIO_IRQ_EDGE_RISE);
    split_i2c_controller.read_ready = true;
#endif
}
#endif

// private functions
static void split_init_i2c_controller(void) {
#ifdef SPLIT_I2C
    gpio_pull_up(SPLIT_I2C_SCL);
    gpio_pull_up(SPLIT_I2C_SDA);

#if defined(SPLIT_INTERRUPT)
    gpio_init(SPLIT_INTERRUPT);
    gpio_set_dir(SPLIT_INTERRUPT, false);
    gpio_add_raw_irq_handler(SPLIT_INTERRUPT, split_target_ready_for_read_irq);
    gpio_set_irq_enabled(SPLIT_INTERRUPT, GPIO_IRQ_EDGE_RISE, true);
#endif
#endif
}

static void split_init_i2c_target(void) {
#ifdef SPLIT_I2C
    i2c_set_slave_mode(SPLIT_I2C, true, SPLIT_I2C_ADDRESS);

    gpio_set_pulls(SPLIT_I2C_SCL, false, false);
    gpio_set_pulls(SPLIT_I2C_SDA, false, false);

    SPLIT_I2C->hw->intr_mask = (
        I2C_IC_INTR_MASK_M_START_DET_BITS       // Start condition                  [SW clears]
        | I2C_IC_INTR_MASK_M_RESTART_DET_BITS   // Restart condition                [SW clears]
        | I2C_IC_INTR_MASK_M_TX_ABRT_BITS       // Read aborted                     [SW clears]
        | I2C_IC_INTR_MASK_M_RD_REQ_BITS        // Read (target -> controller)      [SW clears]
        | I2C_IC_INTR_MASK_M_RX_FULL_BITS       // Write (controller -> target)     [HW clears]
    );

    irq_set_exclusive_handler(SPLIT_I2C_IRQ, split_i2c_target_irq);
    irq_set_enabled(SPLIT_I2C_IRQ, true);

#if defined(SPLIT_INTERRUPT)
    gpio_init(SPLIT_INTERRUPT);
    gpio_set_dir(SPLIT_INTERRUPT, true);
#endif
#endif
}

static void split_init_i2c(void) {
#ifdef SPLIT_I2C
    const uint32_t I2C_PIN_MASK = (1U << SPLIT_I2C_SCL) | (1U << SPLIT_I2C_SDA);
    gpio_init_mask(I2C_PIN_MASK);
    i2c_init(SPLIT_I2C, SPLIT_I2C_BAUDRATE);
    gpio_set_function_masked(I2C_PIN_MASK, GPIO_FUNC_I2C);

#if SPLIT_CONTROLLER
    split_init_i2c_controller();
#else
    split_init_i2c_target();
#endif
#endif
}

static void split_uart_rx_irq(uint8_t byte) {
#if defined(SPLIT_PIO_UART) && defined(SPLIT_CONTROLLER)
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

static void split_init_uart(void) {
#ifdef SPLIT_PIO_UART

#ifdef SPLIT_CONTROLLER
#define SPLIT_UART_TX SPLIT_UART_CONTROLLER_TX
#define SPLIT_UART_RX SPLIT_UART_CONTROLLER_RX
#else
#define SPLIT_UART_TX SPLIT_UART_TARGET_TX
#define SPLIT_UART_RX SPLIT_UART_TARGET_RX
#endif

    pio_uart_init(&split_pio_uart, SPLIT_PIO_UART, SPLIT_UART_TX, SPLIT_UART_RX, SPLIT_UART_BAUDRATE, split_uart_rx_irq);
#endif
}

// public functions
void split_init(void) {
    split_init_i2c();
    split_init_uart();
}

void split_scan_complete(void) {
    // Raise the interrupt if enabled
#ifdef SPLIT_INTERRUPT
    gpio_put(SPLIT_INTERRUPT, true);
#endif

#if defined(SPLIT_PIO_UART)
    pio_uart_write_blocking(&split_pio_uart, sync_seq, sizeof(sync_seq));
    pio_uart_write_blocking(&split_pio_uart, (const uint8_t*)matrix_get_pressed_bitmap(), sizeof(split_uart.keys));
#endif
}

void split_update(void) {
#if defined(SPLIT_I2C) && defined(SPLIT_CONTROLLER)
    if (split_i2c_controller.read_ready || gpio_get(SPLIT_INTERRUPT)) {
        // Read the key data from the target
        int bytes_read = i2c_read_burst_blocking(SPLIT_I2C, SPLIT_I2C_ADDRESS, (uint8_t*)split_i2c_controller.keys, sizeof(uint32_t)*MATRIX_ROWS);
        if (bytes_read < sizeof(uint32_t)*MATRIX_ROWS) {
            memset(split_i2c_controller.keys, 0, sizeof(split_i2c_controller.keys));
        }
        split_i2c_controller.read_ready = false;
    }
#endif
}

uint32_t split_get_target_row(uint row) {
    if (row >= MATRIX_ROWS) {
        return 0;
    }
#ifdef SPLIT_I2C
    return split_i2c_controller.keys[row];
#elif defined(SPLIT_PIO_UART)
    return split_uart.keys[row];
#endif
}
