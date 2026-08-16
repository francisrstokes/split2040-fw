#include "mock_combo.h"
#include "CppUTestExt/MockSupport_c.h"

#define combo_system              prod_combo_system

#include "combo.c"

#undef combo_system

// Mocks
KB_SYSTEM_MOCKS(combo)

// Function pointer structs
static const StCombo_t MockStruct = {
    .combo_system = mock_combo_system,
};

static const StCombo_t ProdStruct = {
    .combo_system = prod_combo_system,
};

static StCombo_t ActiveStruct = MockStruct;

// API
void mock_combo_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        ActiveStruct = ProdStruct;
    }
}
StCombo_t* mock_combo_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

ComboInternals_t* mock_combo_get_internals(void) {
    static ComboInternals_t Internals = {
        .combos = &combos,

        // Private functions
        .combo_get_key_index = combo_get_key_index,
        .combo_find_next_with_key = combo_find_next_with_key,
        .combo_update_key_in_active = combo_update_key_in_active,
        .combo_start = combo_start,
        .combo_is_complete = combo_is_complete,
        .combo_get_single_pressed_index = combo_get_single_pressed_index,
        .combo_mark_keys_as_handled = combo_mark_keys_as_handled,
        .combo_deactivate_unfinished_overlapping_combos = combo_deactivate_unfinished_overlapping_combos,
        KB_SYSTEM_INTERNALS_PRIVATE_FN_ASSIGNMENTS(combo)
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
const keyboard_system_t* combo_system(void) {
    return ActiveStruct.combo_system();
}