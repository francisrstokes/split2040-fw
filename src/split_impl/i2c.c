#include "i2c.h"
#include "../matrix.h"

#include <string.h>

#ifdef SPLIT_I2C

// forwards
static void split_i2c_init(void);
static void split_i2c_update(void);
static uint32_t split_i2c_get_target_row(uint row);
static void split_i2c_scan_complete(void);

// statics
static split_impl_t split_i2c_impl = {
    .init = split_i2c_init,
    .scan_complete = split_i2c_scan_complete,
    .update = split_i2c_update,
    .get_target_row = split_i2c_get_target_row,
};

static split_i2c_target_t split_i2c_target = {
    .state = SPL_I2C_IDLE,
    .address = 0,
    .read_started = false
};

static split_i2c_controller_t split_i2c_controller = {
    .read_ready = false,
    .keys = {0}
};

// irqs
static void split_i2c_target_irq(void) {
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
}

static void split_target_ready_for_read_irq(void) {
    gpio_acknowledge_irq(SPLIT_INTERRUPT, GPIO_IRQ_EDGE_RISE);
    split_i2c_controller.read_ready = true;
}

// private functions
static void split_i2c_init_controller(void) {
    gpio_pull_up(SPLIT_I2C_SCL);
    gpio_pull_up(SPLIT_I2C_SDA);

#if defined(SPLIT_INTERRUPT)
    gpio_init(SPLIT_INTERRUPT);
    gpio_set_dir(SPLIT_INTERRUPT, false);
    gpio_add_raw_irq_handler(SPLIT_INTERRUPT, split_target_ready_for_read_irq);
    gpio_set_irq_enabled(SPLIT_INTERRUPT, GPIO_IRQ_EDGE_RISE, true);
#endif
}

static void split_i2c_init_target(void) {
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
}

static void split_i2c_init(void) {
    const uint32_t I2C_PIN_MASK = (1U << SPLIT_I2C_SCL) | (1U << SPLIT_I2C_SDA);
    gpio_init_mask(I2C_PIN_MASK);
    i2c_init(SPLIT_I2C, SPLIT_I2C_BAUDRATE);
    gpio_set_function_masked(I2C_PIN_MASK, GPIO_FUNC_I2C);

#if SPLIT_CONTROLLER
    split_i2c_init_controller();
#else
    split_i2c_init_target();
#endif
}

static void split_i2c_update(void) {
#if defined(SPLIT_CONTROLLER)
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

static uint32_t split_i2c_get_target_row(uint row) {
    if (row >= MATRIX_ROWS) {
        return 0;
    }
    return split_i2c_controller.keys[row];
}

static void split_i2c_scan_complete(void) {
    // Raise the interrupt if enabled
#ifdef SPLIT_INTERRUPT
    gpio_put(SPLIT_INTERRUPT, true);
#endif
}

// public functions
split_impl_t* split_i2c_get_impl(void) {
    return &split_i2c_impl;
}

#endif
