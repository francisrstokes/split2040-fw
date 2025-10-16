/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// public functions
void mouse_init(mouse_report_t* mouse_report_ref);
bool mouse_on_key_release(uint row, uint col, keymap_entry_t key);
bool mouse_on_key_press(uint row, uint col, keymap_entry_t key);
bool mouse_update(void);
