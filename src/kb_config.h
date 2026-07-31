/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// defines
#define KB_CONFIG_CURRENT_PROTOCOL_VERSION  (1)
#define KB_CONFIG_CURRENT_FORMAT_VERSION    (1)

#define KB_CONFIG_MSG_TYPE_VALUE_MASK       (0x1f)
#define KB_CONFIG_MSG_TYPE_REQ_RES_MASK     (0x80)
#define KB_CONFIG_MSG_TYPE_ACK_MASK         (0x40)

#define KB_CONFIG_MSG_TYPE_REQ              (0x00)
#define KB_CONFIG_MSG_TYPE_RES              (0x80)

#define KB_CONFIG_MSG_GET_INFO              (0x01)
#define KB_CONFIG_MSG_GET_LAYOUT            (0x02)
#define KB_CONFIG_MSG_SET_KEY               (0x03)
#define KB_CONFIG_MSG_COMMIT                (0x04)
#define KB_CONFIG_MSG_GET_MACRO             (0x05)
#define KB_CONFIG_MSG_SET_MACRO             (0x06)
#define KB_CONFIG_MSG_RESET_TO_BL           (0x07)
#define KB_CONFIG_MSG_DUMP_CONFIG           (0x08)
#define KB_CONFIG_MSG_GET_COMBO             (0x09)
#define KB_CONFIG_MSG_SET_COMBO             (0x0A)
#define KB_CONFIG_MSG_GET_RING_BUFFER_DATA  (0x0B)

#define KB_CONFIG_SENTINEL_VALUE            (0x4b454542) // "KEEB"
#define KB_CONFIG_COMMIT_VALUE              (0x434f4f4c) // "COOL"
#define KB_CONFIG_COMMIT_OP_CANCEL          (0)
#define KB_CONFIG_COMMIT_OP_SAVE            (1)
#define KB_CONFIG_COMMIT_OP_ERASE           (2)

// typedefs
typedef void (*kb_config_transfer_complete_cb_t)(void);
typedef struct kb_config_bulk_ptrs_t {
    // Written by kb_config to be used by usb
    kb_config_transfer_complete_cb_t tx_complete;
    kb_config_transfer_complete_cb_t rx_complete;

    // Written by usb to be used by kb_config
    void (*tx)(uint8_t* buf, uint16_t len);
    void (*rx)(uint8_t* buf, uint16_t len);
} kb_config_bulk_ptrs_t;

// 4 byte header
typedef struct kb_config_msg_header_t {
    uint8_t type;                   // Type of message (bit 7 = request (0) or response (1))
    uint8_t packet_number;          // Which packet is this? [packets total = ceil(payload_length / (64 - sizeof(kb_config_msg_header_t)))]
    uint16_t payload_length;        // How many bytes are in the total payload (not including this header)
} __packed kb_config_msg_header_t;

typedef struct kb_config_get_info_t {
    uint8_t protocol_version;       // Which version of the protocol is this? (GET_INFO must always have the same type value, and this field must remain)
    uint8_t row_count;              // Number of rows in the key matrix
    uint8_t column_count;           // Number of columns in the key matrix
    uint8_t layer_count;            // Number of layers for this keyboard
    uint8_t led_count;              // How many (WS2818B) RGB LEDs are onboard
    uint8_t macro_count;            // How many macro slots are available
    uint8_t combo_count;            // How many combo slots are available
    uint8_t macro_max_size;         // Maximum length of a macro string
    uint8_t combo_max_size;         // Maximum number of keys allowed in a combo
} __packed kb_config_get_info_t;

typedef struct kb_config_set_key_t {
    uint8_t layer;
    uint8_t row;
    uint8_t col;
    uint8_t padding;
    uint32_t value;
} __packed kb_config_set_key_t;

typedef struct kb_config_commit_t {
    uint32_t commit_value;
    uint8_t operation;
} __packed kb_config_commit_t;

typedef struct kb_config_macro_t {
    uint16_t macro_type;
    uint16_t length;
    char string[MACRO_SIZE_MAX];
} __packed kb_config_macro_t;

typedef struct kb_config_combo_t {
    uint32_t keys[COMBO_KEYS_MAX];
    uint32_t key_out;
} __packed kb_config_combo_t;

typedef struct kb_config_set_macro_t {
    uint8_t index;
    kb_config_macro_t macro;
} __packed kb_config_set_macro_t;

typedef struct kb_config_set_combo_t {
    uint8_t index;
    kb_config_combo_t combo;
} __packed kb_config_set_combo_t;

typedef struct kb_config_message_state_t {
    bool transmitting;
    kb_config_msg_header_t header;
    uint16_t data_bytes_written;
    const uint8_t* data_buffer;
} kb_config_message_state_t;

typedef struct kb_config_flash_header_t {
    uint32_t sentinel;
    uint32_t format_version;
    uint32_t write_count;

    // Basic info
    uint8_t row_count;
    uint8_t column_count;
    uint8_t layer_count;
    uint8_t led_count;
    uint8_t macro_count;
    uint8_t combo_count;
    uint8_t macro_max_size;
    uint8_t combo_max_size;
} kb_config_flash_header_t;

// public functions
void kb_config_init(void);
void kb_config_reset(void);
kb_config_bulk_ptrs_t* kb_config_get_bulk_ptrs(void);
void kb_config_log_to_ring_buffer(void* data, uint16_t length);
