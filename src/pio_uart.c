// pio_uart.c

#include "pio_uart.h"
#include "pio_uart.pio.h"
#include "hardware/irq.h"

// statics
static pio_uart_t* pio_uart_instances[NUM_PIOS];
static pio_uart_rx_irq_t  pio_uart_rx_irqs[NUM_PIOS];

static void pio_uart_isr(void) {
    for (uint i = 0; i < NUM_PIOS; i++) {
        pio_uart_t *instance = pio_uart_instances[i];
        if (!instance) continue;

        // Drain everything currently in the rx fifo
        while (!pio_sm_is_rx_fifo_empty(instance->pio, instance->sm_rx)) {
            // The rx fifo contains u32 entries, where the most significant byte contains the actual data
            uint8_t byte = pio_sm_get(instance->pio, instance->sm_rx) >> 24u;
            if (pio_uart_rx_irqs[i]) {
                pio_uart_rx_irqs[i](byte);
            }
        }
    }
}

bool pio_uart_init(pio_uart_t *instance, PIO pio, uint pin_tx, uint pin_rx, uint baud, pio_uart_rx_irq_t rx_cb) {
    uint idx = pio_get_index(pio);

    // Attempt to claim for both tx and rx
    int sm_tx = pio_claim_unused_sm(pio, false);
    int sm_rx = pio_claim_unused_sm(pio, false);

    // If either fail, give up
    if (sm_tx < 0 || sm_rx < 0) {
        if (sm_tx >= 0) pio_sm_unclaim(pio, sm_tx);
        if (sm_rx >= 0) pio_sm_unclaim(pio, sm_rx);
        return false;
    }

    // Load the pio programs
    uint off_tx = pio_add_program(pio, &uart_tx_program);
    uint off_rx = pio_add_program(pio, &uart_rx_program);
    uart_tx_program_init(pio, (uint)sm_tx, off_tx, pin_tx, baud);
    uart_rx_program_init(pio, (uint)sm_rx, off_rx, pin_rx, baud);

    // Write data to the instance
    instance->pio = pio;
    instance->sm_tx = (uint)sm_tx;
    instance->sm_rx = (uint)sm_rx;

    // Configure the rx irq
    if (rx_cb) {
        pio_uart_instances[idx] = instance;
        pio_uart_rx_irqs[idx] = rx_cb;

        // Route "RX FIFO not empty, for this SM" onto the PIO block's
        // first internal IRQ line (index 0), which is wired straight
        // through to one NVIC interrupt (PIO0_IRQ_0 or PIO1_IRQ_0).
        pio_set_irqn_source_enabled(pio, 0, pis_sm0_rx_fifo_not_empty + instance->sm_rx, true);

        uint irq_num = (idx == 0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
        // If you're also using this PIO block's IRQ 0 for something
        // else, swap this for irq_add_shared_handler() instead --
        // exclusive handlers will assert if more than one is
        // registered on the same NVIC line.
        irq_set_exclusive_handler(irq_num, pio_uart_isr);
        irq_set_enabled(irq_num, true);
    }

    return true;
}

void pio_uart_write_blocking(pio_uart_t *instance, const uint8_t *data, uint len) {
    for (uint i = 0; i < len; i++) {
        pio_sm_put_blocking(instance->pio, instance->sm_tx, (uint32_t)data[i]);
    }
}
