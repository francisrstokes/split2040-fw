#include "mock_combo.h"
#include "CppUTestExt/MockSupport_c.h"

#define combo_init              prod_combo_init
#define combo_reset             prod_combo_reset
#define combo_on_key_press      prod_combo_on_key_press
#define combo_on_key_release    prod_combo_on_key_release
#define combo_update            prod_combo_update

#include "combo.c"

#undef combo_init
#undef combo_reset
#undef combo_on_key_press
#undef combo_on_key_release
#undef combo_update

// Mocks
static void mock_combo_init(combo_t* combo_table) {
    mock_c()->actualCall("combo_init")
    ->withPointerParameters("combo_table", (void*)combo_table);
}
static void mock_combo_reset(void) {
    mock_c()->actualCall("combo_reset");
}
static bool mock_combo_on_key_press(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("combo_on_key_press")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_combo_on_key_release(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("combo_on_key_release")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_combo_update(void) {
    mock_c()->actualCall("combo_update");
    return mock_c()->returnBoolValueOrDefault(false);
}

// Function pointer structs
static const StCombo_t MockStruct = {
    .combo_init = mock_combo_init,
    .combo_reset = mock_combo_reset,
    .combo_on_key_press = mock_combo_on_key_press,
    .combo_on_key_release = mock_combo_on_key_release,
    .combo_update = mock_combo_update,
};

static const StCombo_t ProdStruct = {
    .combo_init = prod_combo_init,
    .combo_reset = prod_combo_reset,
    .combo_on_key_press = prod_combo_on_key_press,
    .combo_on_key_release = prod_combo_on_key_release,
    .combo_update = prod_combo_update,
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
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
void combo_init(combo_t* combo_table) {
    return ActiveStruct.combo_init(combo_table);
}
void combo_reset(void) {
    return ActiveStruct.combo_reset();
}
bool combo_on_key_press(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.combo_on_key_press(row, col, key);
}
bool combo_on_key_release(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.combo_on_key_release(row, col, key);
}
bool combo_update(void) {
    return ActiveStruct.combo_update();
}
