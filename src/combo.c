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
static int combo_get_key_index(uint combo_index, keymap_entry_t key) {
    for (uint key_index = 0; key_index < COMBO_KEYS_MAX; key_index++) {
        if (combos[combo_index].keys[key_index] == key) return key_index;
        if (combos[combo_index].keys[key_index] == KC_NONE) return -1;
    }
    return -1;
}

static int combo_find_next_with_key(uint start_index, keymap_entry_t key) {
    for (uint i = start_index; i < COMBO_MAX; i++) {
        if (combos[start_index].state == combo_state_invalid) return -1;

        int key_index = combo_get_key_index(i, key);
        if (key_index != -1) {
            return i;
        }
    }
    return -1;
}

static void combo_update_key_in_active(uint combo_index, uint key_index, uint row, uint col) {
    combos[combo_index].keys_pressed_bitmask |= (1 << key_index);
    combos[combo_index].key_positions[key_index].row = row;
    combos[combo_index].key_positions[key_index].col = col;
    matrix_mark_key_as_handled(row, col);
}

static void combo_start(uint combo_index, uint key_index) {
    combos[combo_index].state = combo_state_active;

    // Set all the key positions to 0xff, since 0x00 will always be a valid column and row and could cause misfires
    memset(combos[combo_index].key_positions, 0xff, sizeof(combos[combo_index].key_positions));
    combos[combo_index].time_since_first_press = 0;
    combos[combo_index].keys_pressed_bitmask = 0;
}

static bool combo_is_complete(uint combo_index) {
    for (uint key_index = 0; key_index < COMBO_KEYS_MAX; key_index++) {
        if (combos[combo_index].keys[key_index] == KC_NONE) break;
        if ((combos[combo_index].keys_pressed_bitmask & (1 << key_index)) == 0) {
            return false;
        }
    }
    return true;
}

static int combo_get_single_pressed_index(uint combo_index) {
    uint8_t mask = combos[combo_index].keys_pressed_bitmask;

    for (uint key_index = 0; key_index < COMBO_KEYS_MAX; key_index++) {
        if (combos[combo_index].keys[key_index] == KC_NONE) return -1;

        // This was a pressed key, so could be the only one
        if (mask & (1 << key_index)) {
            // Clear its bit in the mask
            mask &= ~(1 << key_index);

            // If any other bits remain, it wasn't a single press
            if (mask != 0) {
                return -1;
            }

            // Otherwise it was, so this is our index
            return key_index;
        }
    }

    return -1;
}

static void combo_mark_keys_as_handled(uint combo_index) {
    // Mark the keys involved as handled
    for (uint key_index = 0; key_index < COMBO_KEYS_MAX; key_index++) {
        if (combos[combo_index].keys[key_index] == KC_NONE) break;
        matrix_mark_key_as_handled(combos[combo_index].key_positions[key_index].row, combos[combo_index].key_positions[key_index].col);
    }
}

static void combo_deactivate_unfinished_overlapping_combos(uint combo_index) {
    for (uint key_index = 0; key_index < COMBO_KEYS_MAX; key_index++) {
        keymap_entry_t key = combos[combo_index].keys[key_index];
        if (key == KC_NONE) break;

        for (uint other_index = 0; other_index < COMBO_MAX; other_index++) {
            if (other_index == combo_index) continue;
            if (combos[other_index].state == combo_state_invalid) break;

            if (combo_get_key_index(other_index, key) != -1) {
                combos[other_index].state = combo_state_cooldown;
                combos[other_index].time_since_first_press = 0;
                break;
            }
        }
    }
}

// public functions
void combo_init(combo_t* combo_table) {
    combos = combo_table;
}

bool combo_on_key_press(uint row, uint col, keymap_entry_t key) {
    bool was_handled = false;
    int combo_index = combo_find_next_with_key(0, key);

    while (combo_index != -1) {
        int key_index = combo_get_key_index(combo_index, key);

        if (combos[combo_index].state == combo_state_cooldown) {
            // Ignore keys while in cooldown
            was_handled = true;
        } else if (combos[combo_index].state == combo_state_wait_for_all_released) {
            // Just remark the key as pressed
            combos[combo_index].keys_pressed_bitmask |= 1 << key_index;
            was_handled = true;
        } else {
            was_handled = true;

            if (combos[combo_index].state == combo_state_active) {
                combo_update_key_in_active(combo_index, key_index, row, col);
            } else if (combos[combo_index].state == combo_state_inactive) {
                combo_start(combo_index, key_index);
                combo_update_key_in_active(combo_index, key_index, row, col);
            }

            if (combo_is_complete(combo_index)) {
                keyboard_send_key(combos[combo_index].key_out);
                combos[combo_index].state = combo_state_wait_for_all_released;

                // Any combo involving any of these keys also needs to be made in active, to prevent unwanted single presses
                combo_deactivate_unfinished_overlapping_combos(combo_index);
            }
        }

        // Get the next associated combo
        combo_index = combo_find_next_with_key(combo_index + 1, key);
    }

    return was_handled;
}

bool combo_on_key_release(uint row, uint col, keymap_entry_t key) {
    bool was_handled = false;
    int combo_index = combo_find_next_with_key(0, key);

    while (combo_index != -1) {
        int key_index = combo_get_key_index(combo_index, key);

        if (combos[combo_index].state == combo_state_cooldown) {
            // Ignore keys while in cooldown
        } else if (combos[combo_index].state == combo_state_wait_for_all_released) {
            // Unmark this key
            combos[combo_index].keys_pressed_bitmask &= ~(1 << key_index);

            // If all the keys are released this combo can return to inactive
            if (combos[combo_index].keys_pressed_bitmask == 0) {
                combos[combo_index].state = combo_state_inactive;
            }
        } else if (combos[combo_index].state == combo_state_single_held) {
            combos[combo_index].state = combo_state_inactive;
        } else {
            if (combos[combo_index].state == combo_state_active) {
                // When only a single key was pressed, we can emit the key immediately
                int single_key_index = combo_get_single_pressed_index(combo_index);
                if (single_key_index == -1) {
                    // There was more than one key pressed, go to the cooldown state
                    combos[combo_index].state = combo_state_cooldown;
                    combos[combo_index].time_since_first_press = 0;

                    // Unmark this key
                    combos[combo_index].keys_pressed_bitmask &= ~(1 << key_index);
                } else {
                    // It was a single, emit it!
                    keyboard_send_key(combos[combo_index].keys[single_key_index]);
                    combos[combo_index].state = combo_state_inactive;
                }
            } else if (combos[combo_index].state == combo_state_inactive) {
                // Weird to be in this state when a key was released
            }
        }

        // Get the next associated combo
        combo_index = combo_find_next_with_key(combo_index + 1, key);
    }

    return was_handled;
}

bool combo_update(void) {
    bool there_are_unresolved_combos = false;

    for (uint combo_index = 0; combo_index < COMBO_MAX; combo_index++) {
        if (combos[combo_index].state == combo_state_invalid) break;
        if (combos[combo_index].state == combo_state_inactive) continue;

        if (combos[combo_index].state == combo_state_cooldown) {
            // If the configured time has passed, go to inactive
            combos[combo_index].time_since_first_press += MATRIX_SCAN_INTERVAL_MS;
            if (combos[combo_index].time_since_first_press >= COMBO_CANCEL_SUPPRESS_MS) {
                combos[combo_index].state = combo_state_inactive;
            } else {
                combo_mark_keys_as_handled(combo_index);
            }
        } else if (combos[combo_index].state == combo_state_wait_for_all_released) {
            // This is handled by press and release events
            combo_mark_keys_as_handled(combo_index);
        } else if (combos[combo_index].state == combo_state_active) {
            there_are_unresolved_combos = true;

            combos[combo_index].time_since_first_press += MATRIX_SCAN_INTERVAL_MS;
            if (combos[combo_index].time_since_first_press >= COMBO_DELAY_MS) {

                // When only a single key was pressed, we can emit the key immediately
                int single_key_index = combo_get_single_pressed_index(combo_index);
                if (single_key_index == -1) {
                    // There was more than one key pressed, go to the cooldown state
                    combos[combo_index].state = combo_state_cooldown;
                    combos[combo_index].time_since_first_press = 0;
                    combo_mark_keys_as_handled(combo_index);
                } else {
                    // It was a single key, and is still held
                    keyboard_send_key(combos[combo_index].keys[single_key_index]);
                    combos[combo_index].state = combo_state_single_held;
                    combos[combo_index].held_index = single_key_index;
                }
            }
        } else if (combos[combo_index].state == combo_state_single_held) {
            keyboard_send_key(combos[combo_index].keys[combos[combo_index].held_index]);
        }
    }

    return false;
}
