/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

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

// public functions
void kb_config_init(void);
void kb_config_reset(void);
kb_config_bulk_ptrs_t* kb_config_get_bulk_ptrs(void);
