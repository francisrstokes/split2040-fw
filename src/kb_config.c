#include <hardware/flash.h>
#include <pico/bootrom.h>

#include "kb_config.h"
#include "keyboard.h"
#include "macro.h"
#include "combo.h"
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

#define FLASH_KEYMAP_PTR            (void*)(flash_buffer + sizeof(kb_config_flash_header_t))
#define FLASH_MACROS_PTR            (FLASH_KEYMAP_PTR + sizeof(keymap))
#define FLASH_MACRO(index)          (&((kb_config_macro_t*)(FLASH_MACROS_PTR))[index])

#define KEY_LAYER_PTR(layer)        (FLASH_KEYMAP_PTR + ((layer) * MATRIX_ROWS * MATRIX_COLS * sizeof(uint32_t)))
#define KEY_ROW_PTR(layer, row)     (KEY_LAYER_PTR(layer) + ((row) * MATRIX_COLS * sizeof(uint32_t)))
#define KEY_PTR(layer, row, col)    (uint32_t*)(KEY_ROW_PTR(layer, row) + ((col) * sizeof(uint32_t)))

#define FLASH_COMBOS_START           (FLASH_MACROS_PTR + (MACRO_MAX * sizeof(kb_config_macro_t)))
#define FLASH_COMBO(index)           (&((kb_config_combo_t*)FLASH_COMBOS_START)[((index))])

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
    .macro_max_size = MACRO_SIZE_MAX,
    .combo_max_size = COMBO_KEYS_MAX
};

extern const keymap_entry_t keymap[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS];
extern macro_t macros[MACRO_MAX];
extern combo_t combos[COMBO_MAX];

static const uint16_t layout_size = MATRIX_COLS * MATRIX_ROWS * sizeof(uint32_t);

static kb_config_message_state_t message_state = {0};

// symbols provided by linker
extern uint32_t APP_DATA_START_ADDR;

// private functions
static void kb_config_load_from_flash(void) {
    // Load the flash region and check if it's valid
    kb_config_flash_header_t* flash_header = (kb_config_flash_header_t*)&APP_DATA_START_ADDR;
    if (flash_header->sentinel == KB_CONFIG_SENTINEL_VALUE) {
        void* app_data_start = (void*)&APP_DATA_START_ADDR;
        memcpy(flash_buffer, app_data_start, sizeof(flash_buffer));

        // Copy the macros to the main buffer
        for (int i = 0; i < MACRO_MAX; i++) {
            macros[i].type = FLASH_MACRO(i)->macro_type;
            macros[i].send_string.length = FLASH_MACRO(i)->length;
            macros[i].send_string.buffer = FLASH_MACRO(i)->string;
        }

        // Copy the combos to the main buffer
        for (int i = 0; i < COMBO_MAX; i++) {
            combos[i].key_out = FLASH_COMBO(i)->key_out;
            combos[i].state = combos[i].key_out == 0 ? combo_state_invalid : combo_state_inactive;
            memcpy(combos[i].keys, FLASH_COMBO(i)->keys, sizeof(combos[i].keys));
        }
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
            .macro_max_size = MACRO_SIZE_MAX,
            .combo_max_size = COMBO_KEYS_MAX
        };

        // Copy the keymap to the buffer
        memcpy(FLASH_KEYMAP_PTR, keymap, sizeof(keymap));

        // Copy the macro definitions to the buffer
        for (int i = 0; i < MACRO_MAX; i++) {
            FLASH_MACRO(i)->macro_type = macros[i].type;
            FLASH_MACRO(i)->length = macros[i].send_string.length;
            memcpy(FLASH_MACRO(i)->string, macros[i].send_string.buffer, MACRO_SIZE_MAX);
        }

        // Copy the combo definitions to the buffer
        for (int i = 0; i < COMBO_MAX; i++) {
            FLASH_COMBO(i)->key_out = combos[i].key_out;
            memcpy(FLASH_COMBO(i)->keys, combos[i].keys, sizeof(combos[i].keys));
        }
    }

    // Either way, set the keymap to what's in RAM
    keyboard_set_keymap_ptr(FLASH_KEYMAP_PTR);
}

static void kb_config_write_to_flash(void) {
    // Increment the write counter
    ((kb_config_flash_header_t*)flash_buffer)->write_count++;

    // Erase the current flash contents
    flash_range_erase(KB_CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);

    // Program the flash buffer
    flash_range_program(KB_CONFIG_FLASH_OFFSET, flash_buffer, FLASH_SECTOR_SIZE);
}

static void kb_config_erase_from_flash(void) {
    // Erase the current flash contents
    flash_range_erase(KB_CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);

    // Setup a new flash structure in memory
    kb_config_load_from_flash();
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
            message_state.data_buffer = (const uint8_t*)KEY_LAYER_PTR(layer_index);

            kb_config_transmit_message();
            return;
        } break;

        case KB_CONFIG_MSG_SET_KEY: {
            const kb_config_set_key_t* set_key_msg = (const kb_config_set_key_t*)&tmp_rx_buffer[sizeof(kb_config_msg_header_t)];

            if (set_key_msg->row >= MATRIX_ROWS || set_key_msg->col >= MATRIX_COLS || set_key_msg->layer >= LAYER_MAX) break;

            uint32_t* key_ptr = KEY_PTR(set_key_msg->layer, set_key_msg->row, set_key_msg->col);
            *key_ptr = set_key_msg->value;

            has_uncommitted_state = true;
        } break;

        case KB_CONFIG_MSG_COMMIT: {
            const kb_config_commit_t* commit_msg = (const kb_config_commit_t*)&tmp_rx_buffer[sizeof(kb_config_msg_header_t)];

            if (commit_msg->commit_value != KB_CONFIG_COMMIT_VALUE) break;

            if (commit_msg->operation == KB_CONFIG_COMMIT_OP_CANCEL) {
                // Restore the unmodified config
                kb_config_load_from_flash();
            } else if (commit_msg->operation == KB_CONFIG_COMMIT_OP_SAVE && has_uncommitted_state) {
                // Store the new, uncommitted config
                kb_config_write_to_flash();
            } else if (commit_msg->operation == KB_CONFIG_COMMIT_OP_ERASE) {
                // Erase the current config, and return to the original firmware configuration
                kb_config_erase_from_flash();
            }
        } break;

        case KB_CONFIG_MSG_RESET_TO_BL: {
            reset_usb_boot(0, 0);
        } break;

        case KB_CONFIG_MSG_GET_MACRO: {
            const uint8_t macro_index = tmp_rx_buffer[sizeof(kb_config_msg_header_t)];
            if (macro_index >= MACRO_MAX) break;

            message_state.header = (kb_config_msg_header_t) {
                .packet_number = 0,
                .payload_length = sizeof(kb_config_macro_t),
                .type = KB_CONFIG_MSG_GET_MACRO | KB_CONFIG_MSG_TYPE_RES
            };

            message_state.data_bytes_written = 0;
            message_state.data_buffer = (const uint8_t*)FLASH_MACRO(macro_index);

            kb_config_transmit_message();
            return;
        } break;

        case KB_CONFIG_MSG_SET_MACRO: {
            kb_config_set_macro_t* set_macro = (kb_config_set_macro_t*)&tmp_rx_buffer[sizeof(kb_config_msg_header_t)];
            if (set_macro->index >= MACRO_MAX) break;

            *FLASH_MACRO(set_macro->index) = set_macro->macro;

            macros[set_macro->index].type = set_macro->macro.macro_type;
            macros[set_macro->index].send_string.length = set_macro->macro.length;
            macros[set_macro->index].send_string.buffer = FLASH_MACRO(set_macro->index)->string;
        } break;

        case KB_CONFIG_MSG_DUMP_CONFIG: {
            message_state.header = (kb_config_msg_header_t) {
                .packet_number = 0,
                .payload_length = FLASH_SECTOR_SIZE,
                .type = KB_CONFIG_MSG_DUMP_CONFIG | KB_CONFIG_MSG_TYPE_RES
            };
            message_state.data_bytes_written = 0;
            message_state.data_buffer = (const uint8_t*)flash_buffer;

            kb_config_transmit_message();
            return;
        } break;

        case KB_CONFIG_MSG_SET_COMBO: {
            kb_config_set_combo_t* set_combo = (kb_config_set_combo_t*)&tmp_rx_buffer[sizeof(kb_config_msg_header_t)];
            if (set_combo->index >= COMBO_MAX) break;

            *FLASH_COMBO(set_combo->index) = set_combo->combo;

            combos[set_combo->index].key_out = FLASH_COMBO(set_combo->index)->key_out;
            combos[set_combo->index].state = combo_state_inactive;
            memcpy(combos[set_combo->index].keys, FLASH_COMBO(set_combo->index)->keys, sizeof(combos[set_combo->index].keys));
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
