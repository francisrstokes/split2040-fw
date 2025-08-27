/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "combo.h"
#include "matrix.h"

#include <string.h>

// statics
static combo_t* combos = NULL;

// private functions

// public functions
void combo_init(combo_t* combo_table) {
    combos = combo_table;
}

void combo_update(void) {
    keymap_entry_t key = KC_NONE;

    // Update timers, and check if cancelled combos have had all their keys released
    for (uint i = 0; i < NUM_COMBO_SLOTS; i++) {
        // The first invalid combo slot marks the end of defined combos
        if (combos[i].state == combo_state_invalid) break;

        // Update all the active timers, and mark any that have passed as inactive
        if (combos[i].state == combo_state_active) {
            combos[i].time_since_first_press += MATRIX_SCAN_INTERVAL_MS;
            if (combos[i].time_since_first_press >= COMBO_DELAY_MS) {
                combos[i].state = combo_state_cancelled;
            }
        } else if (combos[i].state == combo_state_cancelled) {
            // Have all the keys associated with this combo been released?
            bool all_released = true;
            for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                if (combos[i].keys[key_index] == KC_NONE) break;
                if (matrix_key_pressed(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col, true)) {
                    all_released = false;
                }
            }

            // When all keys have finally been released, go back to inactive
            if (all_released) {
                combos[i].state = combo_state_inactive;
                continue;
            } else {
                for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                    if (combos[i].keys[key_index] == KC_NONE) break;
                    matrix_mark_key_as_handled(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col);
                }
            }
        }
    }

    // Scan over the keys, noting those part of combos
    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // We only care about keys that are pressed on this scan
            if (!matrix_key_pressed_this_scan(row, col)) continue;

            key = keyboard_resolve_key(row, col);

            for (uint i = 0; i < NUM_COMBO_SLOTS; i++) {
                if (combos[i].state == combo_state_invalid) break;
                if (combos[i].state == combo_state_cancelled) continue;

                for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                    // A none key marks the end of the keys involved in this combo
                    if (combos[i].keys[key_index] == KC_NONE) continue;

                    if (key == combos[i].keys[key_index]) {
                        if (combos[i].state == combo_state_inactive) {
                            combos[i].state = combo_state_active;
                            // Set all the key positions to 0xff, since 0x00 will always be a valid column and row and could cause misfires
                            memset(combos[i].key_positions, 0xff, sizeof(combos[i].key_positions));
                            combos[i].time_since_first_press = 0;
                        }

                        combos[i].keys_pressed_bitmask |= (1 << key_index);
                        combos[i].key_positions[key_index].row = row;
                        combos[i].key_positions[key_index].col = col;
                        matrix_mark_key_as_handled(row, col);
                    }
                }
            }
        }
    }

    // Loop through the active combos, and emit any which were fulfilled
    for (uint i = 0; i < NUM_COMBO_SLOTS; i++) {
        if (combos[i].state == combo_state_invalid) break;
        if (combos[i].state != combo_state_active) continue;

        // First of all, were any of the keys that we had marked as pressed released this scan?
        bool cancelled = false;
        for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
            if (combos[i].keys[key_index] == KC_NONE) break;
            if (matrix_key_released_this_scan(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col)) {
                cancelled = true;
            }
        }

        if (cancelled) {
            combos[i].state = combo_state_cancelled;
            continue;
        }

        bool all_keys_pressed = true;
        for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
            if (combos[i].keys[key_index] == KC_NONE) break;

            // Also check handled keys, since any pressed on this scan would have been marked
            if (!matrix_key_pressed(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col, true)) {
                all_keys_pressed = false;
                break;
            }
        }

        // Check if all were pressed, and send combo key
        if (all_keys_pressed) {
            keyboard_send_key(combos[i].key_out);
            combos[i].state = combo_state_cancelled;

            // Mark the keys involved as handled
            for (uint key_index = 0; key_index < MAX_KEYS_PER_COMBO; key_index++) {
                if (combos[i].keys[key_index] == KC_NONE) break;
                matrix_mark_key_as_handled(combos[i].key_positions[key_index].row, combos[i].key_positions[key_index].col);
            }
        }
    }
}
