#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#include <string.h>

#include "split.h"
#include "matrix.h"

// statics
static split_i2c_target_t split_i2c_target = {
    .state = SPL_I2C_IDLE,
    .address = 0,
    .read_started = false
};

static split_i2c_controller_t split_i2c_controller = {
    .read_ready = false,
    .keys = {0}
};

// I2C target IRQ
static void split_i2c_target_irq(void) {
#ifdef SPLIT_I2C
    i2c_hw_t* hw = SPLIT_I2C->hw;
    uint32_t intr_stat = hw->intr_stat;

    if (intr_stat == 0) {
        return;
    }

    bool do_finish_transfer = false;

    // Transmit abort?
    if (intr_stat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        hw->clr_tx_abrt;
        do_finish_transfer = true;
    }

    // Start detect?
    if (intr_stat & I2C_IC_INTR_STAT_R_START_DET_BITS) {
        hw->clr_start_det;
        do_finish_transfer = true;
    }

    // Stop detect?
    if (intr_stat & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        hw->clr_stop_det;
        do_finish_transfer = true;
    }

    // Finished with read or write?
    if (do_finish_transfer && (split_i2c_target.state & SPL_I2C_OPERATION_IN_PROGRESS)) {
        split_i2c_target.state = SPL_I2C_IDLE;
        split_i2c_target.read_started = false;
    }

    // Read operation
    if (intr_stat & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
        hw->clr_rd_req;

        split_i2c_target.state = SPL_I2C_READ_IN_PROGRESS;

        if (!split_i2c_target.read_started) {
            // New read starts from the beginning
            split_i2c_target.address = 0;
            split_i2c_target.read_started = true;

            // Clear the interrupt towards the controller (if enabled)
#ifdef SPLIT_INTERRUPT
            gpio_put(SPLIT_INTERRUPT, false);
#endif
        }

        const uint32_t* keys_bitmap = matrix_get_pressed_bitmap();
        const uint8_t row = split_i2c_target.address / MATRIX_ROWS;
        const uint8_t shift = split_i2c_target.address & 0x3;

        const uint8_t tx_value = (row < MATRIX_ROWS)
            ? (keys_bitmap[row] >> shift) & 0xff
            : 0;

        i2c_write_byte_raw(SPLIT_I2C, tx_value);
        ++split_i2c_target.address;
    }

    // Write operation
    if (intr_stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        split_i2c_target.state = SPL_I2C_WRITE_IN_PROGRESS;

        if (!split_i2c_target.read_started) {
            split_i2c_target.address = i2c_read_byte_raw(SPLIT_I2C);
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

// private functions
static void split_init_i2c_controller(void) {
#ifdef SPLIT_I2C
    gpio_pull_up(SPLIT_I2C_SCL);
    gpio_pull_up(SPLIT_I2C_SDA);

#if defined(SPLIT_INTERRUPT)
    gpio_pull_down(SPLIT_INTERRUPT);
    gpio_add_raw_irq_handler(SPLIT_INTERRUPT, split_target_ready_for_read_irq);
    gpio_set_irq_enabled(SPLIT_INTERRUPT, GPIO_IRQ_EDGE_RISE, true);
#endif
#endif
}

static void split_init_i2c_target(void) {
#ifdef SPLIT_I2C
    i2c_set_slave_mode(SPLIT_I2C, true, SPLIT_I2C_ADDRESS);

    irq_set_exclusive_handler(SPLIT_I2C_IRQ, split_i2c_target_irq);
    irq_set_enabled(SPLIT_I2C_IRQ, true);
#endif
}

static void split_init_i2c(void) {
#ifdef SPLIT_I2C
    const uint32_t I2C_PIN_MASK = (1U << SPLIT_I2C_SCL) | (1U << SPLIT_I2C_SDA);
    i2c_init(SPLIT_I2C, SPLIT_I2C_BAUDRATE);
    gpio_set_function_masked(I2C_PIN_MASK, GPIO_FUNC_I2C);

#if SPLIT_CONTROLLER
    split_init_i2c_controller();
#else
    split_init_i2c_target();
#endif
#endif
}

// public functions
void split_init(void) {
    split_init_i2c();
}

void split_scan_complete(void) {
    // Raise the interrupt if enabled
#ifdef SPLIT_INTERRUPT
    gpio_put(SPLIT_INTERRUPT, true);
#endif
}

void split_update(void) {
#ifdef SPLIT_CONTROLLER
    if (split_i2c_controller.read_ready) {
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
    return split_i2c_controller.keys[row];
}
