#include "taphold.h"
#include "keyboard.h"
#include "matrix.h"

// statics
static taphold_state_t tapholds = {0};

// private functions
static void taphold_handle_presses(void) {
    keymap_entry_t key = KC_NONE;

    for (uint row = 0; row < MATRIX_ROWS; row++) {
        for (uint col = 0; col < MATRIX_COLS; col++) {
            // Skip anything not pressed, or that has already been processed
            if (!matrix_key_pressed(row, col, false)) continue;

            key = keyboard_resolve_key(row, col);
            if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_TAPHOLD) {
                // Create a new taphold node from the pool
                ll_node_t* taphold_node = lla_alloc_tail(&tapholds.allocator);
                if (taphold_node != NULL) {
                    // Initialise the data
                    taphold_data_t* taphold = (taphold_data_t*)taphold_node->data;
                    taphold->hold_counter = 0;
                    taphold->layer = keyboard_get_current_layer();
                    taphold->col = col;
                    taphold->row = row;
                }
                matrix_mark_key_as_handled(row, col);
            }
        }
    }
}

static void taphold_update_active(void) {
    ll_node_t* current_node = tapholds.allocator.active_head;
    taphold_data_t* current_taphold = NULL;
    keymap_entry_t key = KC_NONE;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;
        key = keyboard_resolve_key(current_taphold->row, current_taphold->col);

        // Update the timer
        current_taphold->hold_counter += MATRIX_SCAN_INTERVAL_MS;
        if (current_taphold->hold_counter > TAP_HOLD_DELAY_MS) {
            current_taphold->hold_counter = TAP_HOLD_DELAY_MS;
        }

        // Is this key still being held? If the key was released or the layer changed, we need to stop tracking this taphold
        bool is_pressed = matrix_key_pressed(current_taphold->row, current_taphold->col, false);
        if (is_pressed) {
            // Since we've dealt with the key here, don't process it in further steps
            matrix_mark_key_as_handled(current_taphold->row, current_taphold->col);
        }

        // If the key was released before the hold delay timed out, we need to send the original key and cancel the taphold
        if (!is_pressed || (current_taphold->layer != keyboard_get_current_layer())) {
            if (current_taphold->hold_counter < TAP_HOLD_DELAY_MS) {
                keyboard_send_key(key);
            }

            ll_node_t* next_current = current_node->next;
            lla_free(&tapholds.allocator, current_node);
            current_node = next_current;

            continue;
        }

        // If we got here, the key is still held. Check if it's active, and use it's modifiers if so
        if (current_taphold->hold_counter >= TAP_HOLD_DELAY_MS) {
            keyboard_send_modifiers(ENTRY_ARG4(key));
            keyboard_send_key(ENTRY_ARG8(key));
        }

        // On to the next
        current_node = current_node->next;
    }
}

static bool taphold_keyboard_should_ignore_other_keys(void) {
    ll_node_t* current_node = tapholds.allocator.active_head;
    taphold_data_t* current_taphold = NULL;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;
        if (current_taphold->hold_counter < TAP_HOLD_DELAY_MS) {
            return true;
        }
        current_node = current_node->next;
        continue;
    }

    return false;
}

// public functions
void taphold_init(void) {
    lla_init(
        &tapholds.allocator,
        tapholds.data_array,
        tapholds.node_array,
        MAX_CONCURRENT_TAPHOLDS,
        sizeof(taphold_data_t)
    );
}

bool taphold_update(void) {
    taphold_update_active();
    taphold_handle_presses();
    return taphold_keyboard_should_ignore_other_keys();
}
