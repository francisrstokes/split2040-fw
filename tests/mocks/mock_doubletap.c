#include "mock_doubletap.h"
#include "CppUTestExt/MockSupport_c.h"

#define double_tap_init              prod_double_tap_init
#define double_tap_reset             prod_double_tap_reset
#define double_tap_update            prod_double_tap_update
#define double_tap_on_key_release    prod_double_tap_on_key_release
#define double_tap_on_key_press      prod_double_tap_on_key_press

#include "doubletap.c"

#undef double_tap_init
#undef double_tap_reset
#undef double_tap_update
#undef double_tap_on_key_release
#undef double_tap_on_key_press

// Mocks
static void mock_double_tap_init(void) {
    mock_c()->actualCall("double_tap_init");
}
static void mock_double_tap_reset(void) {
    mock_c()->actualCall("double_tap_reset");
}
static bool mock_double_tap_update(void) {
    mock_c()->actualCall("double_tap_update");
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_double_tap_on_key_release(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("double_tap_on_key_release")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_double_tap_on_key_press(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("double_tap_on_key_press")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}

// Function pointer structs
static const StDoubleTap_t MockStruct = {
    .double_tap_init = mock_double_tap_init,
    .double_tap_reset = mock_double_tap_reset,
    .double_tap_update = mock_double_tap_update,
    .double_tap_on_key_release = mock_double_tap_on_key_release,
    .double_tap_on_key_press = mock_double_tap_on_key_press,
};

static const StDoubleTap_t ProdStruct = {
    .double_tap_init = prod_double_tap_init,
    .double_tap_reset = prod_double_tap_reset,
    .double_tap_update = prod_double_tap_update,
    .double_tap_on_key_release = prod_double_tap_on_key_release,
    .double_tap_on_key_press = prod_double_tap_on_key_press,
};

static StDoubleTap_t ActiveStruct = MockStruct;

// API
void mock_double_tap_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        ActiveStruct = ProdStruct;
    }
}
StDoubleTap_t* mock_double_tap_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

DoubleTapInternals_t* mock_double_tap_get_internals(void) {
    static DoubleTapInternals_t Internals = {
        .double_taps = &double_taps,

        // Private functions
        .double_tap_is_matching_key = double_tap_is_matching_key,
        .double_tap_find_active = double_tap_find_active,
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
void double_tap_init(void) {
    return ActiveStruct.double_tap_init();
}
void double_tap_reset(void) {
    return ActiveStruct.double_tap_reset();
}
bool double_tap_update(void) {
    return ActiveStruct.double_tap_update();
}
bool double_tap_on_key_release(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.double_tap_on_key_release(row, col, key);
}
bool double_tap_on_key_press(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.double_tap_on_key_press(row, col, key);
}