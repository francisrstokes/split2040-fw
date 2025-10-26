#include "kb_config.h"
#include "leds.h"

// forward declarations
static void kb_config_rx_complete(void);
static void kb_config_tx_complete(void);

// statics
static uint8_t tmp_rx_buffer[64] = {0};
static uint8_t tmp_tx_buffer[64] = {0};
static uint8_t flash_buffer[4096] = {0};

static kb_config_bulk_ptrs_t bulk_ptrs = {
    .rx_complete = kb_config_rx_complete,
    .tx_complete = kb_config_tx_complete,
    .tx = NULL,
    .rx = NULL,
};

// private functions
static void kb_config_rx_complete(void) {
    leds_toggle_debug_led();

    // Queue the next buffer
    bulk_ptrs.rx(tmp_rx_buffer, 64);
}

static void kb_config_tx_complete(void) {
    // TODO
}

// public functions
void kb_config_init(void) {
    // Queue the reception of a packet
    bulk_ptrs.rx(tmp_rx_buffer, 64);
}

void kb_config_reset(void) {

}

kb_config_bulk_ptrs_t* kb_config_get_bulk_ptrs(void) {
    return &bulk_ptrs;
}
