#include "taphold.h"
#include "keyboard.h"
#include "matrix.h"

// defines
#define MAX_NUM_HOLD_TIME_OFFSETS   (10)

// typedefs
typedef struct taphold_hold_time_offset_t {
    keymap_entry_t key;
    int16_t time_offset;
} taphold_hold_time_offset_t;

// statics
static taphold_state_t tapholds = {0};
static const taphold_hold_time_offset_t time_offsets[MAX_NUM_HOLD_TIME_OFFSETS] = {
    [0] = { KC_D,     -20},
    [1] = { KC_K,     -20},
    [2] = { KC_A,     +100},
    [3] = { KC_L,     +20},
    [4] = { KC_S,     +20},
    [5] = { 0 },
    [6] = { 0 },
    [7] = { 0 },
    [8] = { 0 },
    [9] = { 0 }
};

// private functions
static int16_t taphold_get_time_offset_for_key(keymap_entry_t key) {
    for (uint i = 0; i < MAX_NUM_HOLD_TIME_OFFSETS; i++) {
        if (time_offsets[i].key == key) {
            return time_offsets[i].time_offset;
        }
    }
    return 0;
}

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
    uint16_t key_time_offset = 0;
    uint16_t computed_hold_time = TAP_HOLD_DELAY_MS;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;
        key = keyboard_resolve_key(current_taphold->row, current_taphold->col);
        key_time_offset = taphold_get_time_offset_for_key(key);
        computed_hold_time = TAP_HOLD_DELAY_MS + key_time_offset;

        // Update the timer
        current_taphold->hold_counter += MATRIX_SCAN_INTERVAL_MS;
        if (current_taphold->hold_counter > computed_hold_time) {
            current_taphold->hold_counter = computed_hold_time;
        }

        // Is this key still being held? If the key was released or the layer changed, we need to stop tracking this taphold
        bool is_pressed = matrix_key_pressed(current_taphold->row, current_taphold->col, false);
        if (is_pressed) {
            // Since we've dealt with the key here, don't process it in further steps
            matrix_mark_key_as_handled(current_taphold->row, current_taphold->col);
        }

        // If the key was released before the hold delay timed out, we need to send the original key and cancel the taphold
        if (!is_pressed || (current_taphold->layer != keyboard_get_current_layer())) {
            if (current_taphold->hold_counter < computed_hold_time) {
                keyboard_send_key(key);
            }

            ll_node_t* next_current = current_node->next;
            lla_free(&tapholds.allocator, current_node);
            current_node = next_current;

            continue;
        }

        // If we got here, the key is still held. Check if it's active, and use it's modifiers if so
        if (current_taphold->hold_counter >= computed_hold_time) {
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
