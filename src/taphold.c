#include "taphold.h"
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
    [0] = { KC_D,     -50},
    [1] = { KC_K,     -50},
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
    ll_node_t* current_node = tapholds.allocator.active_head;
    taphold_data_t* current_taphold = NULL;
    keymap_entry_t key = KC_NONE;
    uint16_t key_time_offset = 0;
    uint16_t computed_hold_time = TAP_HOLD_DELAY_MS;
    bool there_are_active_undetermined_tapholds = false;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;
        key = keyboard_resolve_key(current_taphold->row, current_taphold->col);
        key_time_offset = taphold_get_time_offset_for_key(key);
        computed_hold_time = TAP_HOLD_DELAY_MS + key_time_offset;

        // Update the timer
        current_taphold->hold_counter += MATRIX_SCAN_INTERVAL_MS;
        if (current_taphold->hold_counter > computed_hold_time) {
            current_taphold->hold_counter = computed_hold_time;
            keyboard_send_key(ENTRY_ARG8(key) | (ENTRY_ARG4(key) << 8));
        } else {
            there_are_active_undetermined_tapholds = true;
        }

        // On to the next
        current_node = current_node->next;
    }

    return there_are_active_undetermined_tapholds;
}

bool taphold_on_key_release(uint row, uint col, keymap_entry_t key) {
    ll_node_t* current_node = tapholds.allocator.active_head;
    taphold_data_t* current_taphold = NULL;
    keymap_entry_t tap_key = KC_NONE;
    uint16_t key_time_offset = 0;
    uint16_t computed_hold_time = TAP_HOLD_DELAY_MS;
    bool key_handled = false;

    while (current_node != NULL) {
        current_taphold = (taphold_data_t*)current_node->data;
        tap_key = keyboard_resolve_key(current_taphold->row, current_taphold->col);

        // Was the tap key released?
        if ((tap_key == key) && (row == current_taphold->row) && (col == current_taphold->col)) {
            key_handled = true;
            key_time_offset = taphold_get_time_offset_for_key(tap_key);
            computed_hold_time = TAP_HOLD_DELAY_MS + key_time_offset;

            // Is it still within the tapping period?
            if (current_taphold->hold_counter < computed_hold_time) {
                // It was, send the key data
                keyboard_send_key(tap_key & 0xfff);
            }

            // Either way, the key is released, so we can free this node
            ll_node_t* next_current = current_node->next;
            lla_free(&tapholds.allocator, current_node);
            current_node = next_current;
            continue;
        }

        // On to the next
        current_node = current_node->next;
    }

    return key_handled;
}

bool taphold_on_key_press(uint row, uint col, keymap_entry_t key) {
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
        return true;
    }
    return false;
}

bool tapholds_any_active(void) {
    return (tapholds.allocator.active_head != NULL);
}
