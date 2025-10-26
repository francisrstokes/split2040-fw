#include <hardware/flash.h>

#include "kb_config.h"
#include "keyboard.h"
#include "leds.h"

#include <string.h>

// forward declarations
static void kb_config_rx_complete(void);
static void kb_config_tx_complete(void);

// defines
#define PACKET_SIZE                 (64)
#define PAYLOAD_SIZE                (64 - sizeof(kb_config_msg_header_t))
#define ROUND_TO_PACKET_SIZE(n)     (((n) + PACKET_SIZE-1) & ~(PACKET_SIZE-1))

#define SECTORS_PER_PAGE            (FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE)
#define KB_CONFIG_FLASH_OFFSET      (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

// statics
static uint8_t tmp_rx_buffer[PACKET_SIZE] = {0};
static uint8_t working_rx_buffer[PACKET_SIZE] = {0};
static uint8_t tmp_tx_buffer[PACKET_SIZE] = {0};
static uint8_t flash_buffer[FLASH_SECTOR_SIZE] = {0};
static bool has_uncommitted_state = false;

static kb_config_bulk_ptrs_t bulk_ptrs = {
    .rx_complete = kb_config_rx_complete,
    .tx_complete = kb_config_tx_complete,
    .tx = NULL,
    .rx = NULL,
};

static const kb_config_get_info_t get_info = {
    .protocol_version = 1,
    .column_count = MATRIX_COLS,
    .row_count = MATRIX_ROWS,
    .layer_count = LAYER_MAX,
    .led_count = LEDS_MAX,
    .macro_count = MACRO_MAX,
    .combo_count = COMBO_MAX,
    .macro_max_size = 32,
    .combo_max_size = COMBO_KEYS_MAX
};

extern const keymap_entry_t keymap[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS];
static const uint16_t layout_size = MATRIX_COLS * MATRIX_ROWS * sizeof(uint32_t);

static kb_config_message_state_t message_state = {0};

// symbols provided by linker
extern uint32_t APP_DATA_START_ADDR;

// private functions
static void kb_config_load_from_flash(void) {
    // Load the flash region and check if it's valid
    kb_config_flash_header_t* flash_header = (kb_config_flash_header_t*)&APP_DATA_START_ADDR;
    if (flash_header->sentinel == KB_CONFIG_SENTINEL_VALUE) {
        memcpy(flash_buffer, &APP_DATA_START_ADDR, sizeof(flash_buffer));
    } else {
        // There is no valid structure in flash. Create on in RAM ready to be written if needed
        *((kb_config_flash_header_t*)flash_buffer) = (kb_config_flash_header_t) {
            .sentinel       = KB_CONFIG_SENTINEL_VALUE,
            .format_version = KB_CONFIG_CURRENT_FORMAT_VERSION,
            .write_count    = 0,
            .column_count   = MATRIX_COLS,
            .row_count      = MATRIX_ROWS,
            .layer_count    = LAYER_MAX,
            .led_count      = LEDS_MAX,
            .macro_count    = MACRO_MAX,
            .combo_count    = COMBO_MAX,
            .macro_max_size = 32,
            .combo_max_size = COMBO_KEYS_MAX
        };

        memcpy(flash_buffer + sizeof(kb_config_flash_header_t), keymap, sizeof(keymap));

        keyboard_set_keymap_ptr(flash_buffer + sizeof(kb_config_flash_header_t));
    }
}

static void kb_config_write_to_flash(void) {
    // Increment the write counter
    ((kb_config_flash_header_t*)flash_buffer)->write_count++;

    // Erase the current flash contents
    flash_range_erase(KB_CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);

    // Program the flash buffer
    flash_range_program(KB_CONFIG_FLASH_OFFSET, flash_buffer, FLASH_SECTOR_SIZE);
}

static void kb_config_transmit_message(void) {
    // Prepare the transmission
    uint16_t bytes_to_send = MIN((message_state.header.payload_length - message_state.data_bytes_written), PAYLOAD_SIZE);
    memcpy(tmp_tx_buffer, &message_state.header, sizeof(kb_config_msg_header_t));
    memcpy(tmp_tx_buffer + sizeof(kb_config_msg_header_t), message_state.data_buffer, bytes_to_send);

    message_state.data_bytes_written += bytes_to_send;
    message_state.data_buffer += bytes_to_send;
    ++message_state.header.packet_number;

    message_state.transmitting = true;
    bulk_ptrs.tx(tmp_tx_buffer, PACKET_SIZE);
}

static void kb_config_update(void) {
    if (message_state.transmitting) {
        if (message_state.data_bytes_written == message_state.header.payload_length) {
            message_state.transmitting = false;
            bulk_ptrs.rx(tmp_rx_buffer, PACKET_SIZE);
        } else {
            kb_config_transmit_message();
        }
    }
}

static void kb_config_rx_complete(void) {
    memcpy(working_rx_buffer, tmp_rx_buffer, PACKET_SIZE);

    uint8_t msg_type = working_rx_buffer[0] & KB_CONFIG_MSG_TYPE_VALUE_MASK;
    switch (msg_type) {
        case KB_CONFIG_MSG_GET_INFO: {
            message_state.header = (kb_config_msg_header_t) {
                .packet_number = 0,
                .payload_length = sizeof(get_info),
                .type = KB_CONFIG_MSG_GET_INFO | KB_CONFIG_MSG_TYPE_RES
            };
            message_state.data_bytes_written = 0;
            message_state.data_buffer = (const uint8_t*)&get_info;

            kb_config_transmit_message();
            return;
        } break;

        case KB_CONFIG_MSG_GET_LAYOUT: {
            const uint8_t layer_index = tmp_rx_buffer[sizeof(kb_config_msg_header_t)];
            if (layer_index >= LAYER_MAX) break;

            message_state.header = (kb_config_msg_header_t) {
                .packet_number = 0,
                .payload_length = layout_size,
                .type = KB_CONFIG_MSG_GET_LAYOUT | KB_CONFIG_MSG_TYPE_RES
            };
            message_state.data_bytes_written = 0;
            message_state.data_buffer = (const uint8_t*)keymap[layer_index];

            kb_config_transmit_message();
            return;
        } break;

        case KB_CONFIG_MSG_SET_KEY: {
            const kb_config_set_key_t* set_key_msg = (const kb_config_set_key_t*)&tmp_rx_buffer[sizeof(kb_config_msg_header_t)];

            if (set_key_msg->row >= MATRIX_ROWS || set_key_msg->col >= MATRIX_COLS || set_key_msg->layer >= LAYER_MAX) break;

            uint32_t (*flash_key_ptr)[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS] = flash_buffer + sizeof(kb_config_flash_header_t);
            (*flash_key_ptr)[set_key_msg->layer][set_key_msg->row][set_key_msg->col] = set_key_msg->value;

            has_uncommitted_state = true;
        } break;

        case KB_CONFIG_MSG_COMMIT: {
            const kb_config_commit_t* commit_msg = (const kb_config_commit_t*)&tmp_rx_buffer[sizeof(kb_config_msg_header_t)];

            if (!has_uncommitted_state || commit_msg->commit_value != KB_CONFIG_COMMIT_VALUE) break;

            if (commit_msg->operation == KB_CONFIG_COMMIT_OP_CANCEL) {
                kb_config_load_from_flash();
            } else if (commit_msg->operation == KB_CONFIG_COMMIT_OP_SAVE) {
                kb_config_write_to_flash();
            }
        } break;
    }

    // If we get here, no messages we're processed, or there's more data to come. Queue the next rx
    bulk_ptrs.rx(tmp_rx_buffer, PACKET_SIZE);
}

static void kb_config_tx_complete(void) {
    kb_config_update();
}

// public functions
void kb_config_init(void) {
    kb_config_load_from_flash();

    // Queue the reception of a packet
    bulk_ptrs.rx(tmp_rx_buffer, PACKET_SIZE);
}

void kb_config_reset(void) {

}

kb_config_bulk_ptrs_t* kb_config_get_bulk_ptrs(void) {
    return &bulk_ptrs;
}
