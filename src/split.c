#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#include "split.h"
#include "split_impl/i2c.h"
#include "split_impl/pio_uart.h"

// statics
static split_impl_t* split_impl = NULL;

// public functions
void split_init(void) {
#if defined(SPLIT_I2C)
    split_impl = split_i2c_get_impl();
#endif

#if defined(SPLIT_PIO_UART)
    split_impl = split_pio_uart_get_impl();
#endif

    if (split_impl != NULL) {
        split_impl->init();
    }
}

void split_scan_complete(void) {
    if (split_impl != NULL) {
        split_impl->scan_complete();
    }
}

void split_update(void) {
    if (split_impl != NULL) {
        split_impl->update();
    }
}

uint32_t split_get_target_row(uint row) {
    if (split_impl != NULL) {
        return split_impl->get_target_row(row);
    }
    return 0;
}
