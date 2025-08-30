/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "doubletap.h"
#include "matrix.h"

// statics
static double_tap_state_t double_taps = {0};

// private functions
static bool double_tap_is_matching_key(double_tap_data_t* dt, keymap_entry_t key) {
    return keyboard_resolve_key_on_layer(dt->row, dt->col, dt->layer) == key;
}

static ll_node_t* double_tap_find_active(keymap_entry_t key) {
    ll_node_t* dt_node = double_taps.allocator.active_head;
    double_tap_data_t* current_dt = NULL;

    while (dt_node != NULL) {
        current_dt = (double_tap_data_t*)dt_node->data;
        if (double_tap_is_matching_key(current_dt, key)) {
            return dt_node;
        }

        // On to the next
        dt_node = dt_node->next;
    }

    return NULL;
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
    ll_node_t* dt_node = double_taps.allocator.active_head;
    double_tap_data_t* current_dt = NULL;
    keymap_entry_t key = KC_NONE;
    bool timer_expired = false;
    bool node_became_inactive = false;
    bool there_are_active_undetermined_double_taps = false;

    while (dt_node != NULL) {
        node_became_inactive = false;
        current_dt = (double_tap_data_t*)dt_node->data;
        key = keyboard_resolve_key_on_layer(current_dt->row, current_dt->col, current_dt->layer);

        // Update the timer
        current_dt->time_since_first_tap += MATRIX_SCAN_INTERVAL_MS;
        bool timer_expired = current_dt->time_since_first_tap >= DOUBLE_TAP_DELAY_MS;
        if (timer_expired) {
            current_dt->time_since_first_tap = DOUBLE_TAP_DELAY_MS;

            // If the state isn't yet resolved, then it's a single tap
            if (current_dt->state != dt_state_double_tap) {
                // If the time expires while waiting for the second tap, we should only send one keydown event
                node_became_inactive = current_dt->state == dt_state_wait_second_press;

                current_dt->state = dt_state_single_tap;
            }
        } else {
            there_are_active_undetermined_double_taps = true;
        }

        if (current_dt->state == dt_state_single_tap) {
            keyboard_send_key(key & 0xfff);
        }

        if (current_dt->state == dt_state_double_tap) {
            keyboard_send_key((ENTRY_ARG4(key) << KEY_MODS_SHIFT ) | ENTRY_ARG8(key));
        }

        if (node_became_inactive) {
            ll_node_t* next_node = dt_node->next;
            lla_free(&double_taps.allocator, dt_node);
            dt_node = next_node;
            continue;
        }

        // On to the next
        dt_node = dt_node->next;
    }

    return there_are_active_undetermined_double_taps;
}

bool double_tap_on_key_release(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_DOUBLE_TAP) {
        ll_node_t* dt_node = double_tap_find_active(key);
        if (dt_node == NULL) return false;

        double_tap_data_t* current_dt = (double_tap_data_t*)dt_node->data;
        if (current_dt->state == dt_state_wait_first_release) {
            current_dt->state = dt_state_wait_second_press;
            return true;
        }

        if (current_dt->state == dt_state_double_tap || current_dt->state == dt_state_single_tap) {
            lla_free(&double_taps.allocator, dt_node);
        }
    }

    return false;
}

bool double_tap_on_key_press(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_DOUBLE_TAP) {
        ll_node_t* dt_node = double_tap_find_active(key);
        if (dt_node == NULL) {
            // Create a new double tap node from the pool
            dt_node = lla_alloc_tail(&double_taps.allocator);
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
            return true;
        }

        double_tap_data_t* current_dt = (double_tap_data_t*)dt_node->data;
        if (current_dt->state == dt_state_wait_second_press) {
            current_dt->state = dt_state_double_tap;
            return true;
        }
    }

    return false;
}
