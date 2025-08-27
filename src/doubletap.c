/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "doubletap.h"
#include "keyboard.h"
#include "matrix.h"

// statics
static double_tap_state_t double_taps = {0};

// private functions
static void double_tap_handle_presses(void) {
    ll_node_t* current_node = double_taps.allocator.active_head;
    double_tap_data_t* current_dt = NULL;
    keymap_entry_t key = KC_NONE;
    bool timer_expired = false;
    bool node_became_inactive = false;

    while (current_node != NULL) {
        current_dt = (double_tap_data_t*)current_node->data;

        // Update the timer
        current_dt->time_since_first_tap += MATRIX_SCAN_INTERVAL_MS;
        bool timer_expired = current_dt->time_since_first_tap >= DOUBLE_TAP_DELAY_MS;
        if (timer_expired) {
            current_dt->time_since_first_tap = DOUBLE_TAP_DELAY_MS;
        }

        bool key_is_pressed = matrix_key_pressed(current_dt->row, current_dt->col, true);

        // Handle the states where the outcome is not yet known
        if (current_dt->state == dt_state_wait_first_release) {
            if (timer_expired) {
                current_dt->state = dt_state_single_tap;
            } else if (!key_is_pressed) {
                current_dt->state = dt_state_wait_second_press;
            }
        } else if (current_dt->state == dt_state_wait_second_press) {
            if (timer_expired) {
                current_dt->state = dt_state_single_tap;
            } else if (key_is_pressed) {
                current_dt->state = dt_state_double_tap;
            }
        }

        // Handle taps and double taps (note: this accounts for state changes that occurred on this tick)
        key = keyboard_resolve_key(current_dt->row, current_dt->col);
        if (current_dt->state == dt_state_single_tap) {
            keyboard_send_key(key);
            node_became_inactive = !key_is_pressed;
        } else if (current_dt->state == dt_state_double_tap) {
            keyboard_send_key((ENTRY_ARG4(key) << KEY_MODS_SHIFT ) | ENTRY_ARG8(key));
            node_became_inactive = !key_is_pressed;
        }

        if (node_became_inactive) {
            ll_node_t* next_current = current_node->next;
            lla_free(&double_taps.allocator, current_node);
            current_node = next_current;
            continue;
        }

        // On to the next
        current_node = current_node->next;
    }
}

static void double_tap_update_active(void) {
    keymap_entry_t key = KC_NONE;

    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            // If this key is transparent, use the base layer instead
            key = keyboard_resolve_key(row, col);

            if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_DOUBLE_TAP) {
                // Create a new double tap node from the pool
                ll_node_t* dt_node = lla_alloc_tail(&double_taps.allocator);
                if (dt_node != NULL) {
                    // Initialise the data
                    double_tap_data_t* dt = (double_tap_data_t*)dt_node->data;
                    dt->time_since_first_tap = 0;
                    dt->layer = keyboard_get_current_layer();
                    dt->col = col;
                    dt->row = row;
                    dt->state = dt_state_wait_first_release;
                }
                matrix_mark_key_as_handled(row, col);
            }
        }
    }
}

static bool double_tap_keyboard_should_ignore_other_keys(void) {
    ll_node_t* current_node = double_taps.allocator.active_head;
    double_tap_data_t* current_dt = NULL;

    while (current_node != NULL) {
        current_dt = (double_tap_data_t*)current_node->data;
        if (current_dt->time_since_first_tap < DOUBLE_TAP_DELAY_MS) {
            return true;
        }
        current_node = current_node->next;
        continue;
    }

    return false;
}

// public functions
void double_tap_init(void) {
    lla_init(
        &double_taps.allocator,
        double_taps.data_array,
        double_taps.node_array,
        MAX_CONCURRENT_DOUBLE_TAPS,
        sizeof(double_tap_data_t)
    );
}

bool double_tap_update(void) {
    double_tap_update_active();
    double_tap_handle_presses();
    return double_tap_keyboard_should_ignore_other_keys();
}
