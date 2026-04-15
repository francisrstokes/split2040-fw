#include "mock_macro.h"
#include "CppUTestExt/MockSupport_c.h"

#define macro_init              prod_macro_init
#define macro_reset             prod_macro_reset
#define macro_on_key_press      prod_macro_on_key_press
#define macro_on_key_release    prod_macro_on_key_release
#define macro_on_virtual_key    prod_macro_on_virtual_key
#define macro_update            prod_macro_update
#define macro_any_active        prod_macro_any_active

#include "macro.c"

#undef macro_init
#undef macro_reset
#undef macro_on_key_press
#undef macro_on_key_release
#undef macro_on_virtual_key
#undef macro_update
#undef macro_any_active

// Mocks
static void mock_macro_init(macro_t* macro_table) {
    mock_c()->actualCall("macro_init")
    ->withPointerParameters("macro_table", (void*)macro_table);
}
static void mock_macro_reset(void) {
    mock_c()->actualCall("macro_reset");
}
static bool mock_macro_on_key_press(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("macro_on_key_press")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_macro_on_key_release(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("macro_on_key_release")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_macro_on_virtual_key(keymap_entry_t key) {
    mock_c()->actualCall("macro_on_virtual_key")
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_macro_update(void) {
    mock_c()->actualCall("macro_update");
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_macro_any_active(void) {
    mock_c()->actualCall("macro_any_active");
    return mock_c()->returnBoolValueOrDefault(false);
}

// Function pointer structs
static const StMacro_t MockStruct = {
    .macro_init = mock_macro_init,
    .macro_reset = mock_macro_reset,
    .macro_on_key_press = mock_macro_on_key_press,
    .macro_on_key_release = mock_macro_on_key_release,
    .macro_on_virtual_key = mock_macro_on_virtual_key,
    .macro_update = mock_macro_update,
    .macro_any_active = mock_macro_any_active,
};

static const StMacro_t ProdStruct = {
    .macro_init = prod_macro_init,
    .macro_reset = prod_macro_reset,
    .macro_on_key_press = prod_macro_on_key_press,
    .macro_on_key_release = prod_macro_on_key_release,
    .macro_on_virtual_key = prod_macro_on_virtual_key,
    .macro_update = prod_macro_update,
    .macro_any_active = prod_macro_any_active,
};

static StMacro_t ActiveStruct = MockStruct;

// API
void mock_macro_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        ActiveStruct = ProdStruct;
    }
}
StMacro_t* mock_macro_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

MacroInternals_t* mock_macro_get_internals(void) {
    static MacroInternals_t Internals = {
        .macros = &macros,
        .ascii_to_hid_kc = &ascii_to_hid_kc,
        .any_macro_active = &any_macro_active,
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
void macro_init(macro_t* macro_table) {
    return ActiveStruct.macro_init(macro_table);
}
void macro_reset(void) {
    return ActiveStruct.macro_reset();
}
bool macro_on_key_press(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.macro_on_key_press(row, col, key);
}
bool macro_on_key_release(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.macro_on_key_release(row, col, key);
}
bool macro_on_virtual_key(keymap_entry_t key) {
    return ActiveStruct.macro_on_virtual_key(key);
}
bool macro_update(void) {
    return ActiveStruct.macro_update();
}
bool macro_any_active(void) {
    return ActiveStruct.macro_any_active();
}
