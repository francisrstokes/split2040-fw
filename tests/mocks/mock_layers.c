#include "mock_layers.h"
#include "CppUTestExt/MockSupport_c.h"

#define layers_reset             prod_layers_reset
#define layers_on_key_press      prod_layers_on_key_press
#define layers_on_key_release    prod_layers_on_key_release
#define layers_on_virtual_key    prod_layers_on_virtual_key
#define layers_get_current       prod_layers_get_current
#define layers_get_base          prod_layers_get_base
#define layers_set               prod_layers_set

#include "layers.c"

#undef layers_reset
#undef layers_on_key_press
#undef layers_on_key_release
#undef layers_on_virtual_key
#undef layers_get_current
#undef layers_get_base
#undef layers_set

// Mocks
static void mock_layers_reset(void) {
    mock_c()->actualCall("layers_reset");
}
static bool mock_layers_on_key_press(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("layers_on_key_press")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_layers_on_key_release(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("layers_on_key_release")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_layers_on_virtual_key(keymap_entry_t key) {
    mock_c()->actualCall("layers_on_virtual_key")
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static uint8_t mock_layers_get_current(void) {
    mock_c()->actualCall("layers_get_current");
    return (uint8_t)(mock_c()->returnUnsignedIntValueOrDefault(0));
}
static uint8_t mock_layers_get_base(void) {
    mock_c()->actualCall("layers_get_base");
    return (uint8_t)(mock_c()->returnUnsignedIntValueOrDefault(0));
}
static void mock_layers_set(uint8_t layer) {
    mock_c()->actualCall("layers_set")
    ->withUnsignedIntParameters("layer", layer);
}

// Function pointer structs
static const StLayers_t MockStruct = {
    .layers_reset = mock_layers_reset,
    .layers_on_key_press = mock_layers_on_key_press,
    .layers_on_key_release = mock_layers_on_key_release,
    .layers_on_virtual_key = mock_layers_on_virtual_key,
    .layers_get_current = mock_layers_get_current,
    .layers_get_base = mock_layers_get_base,
    .layers_set = mock_layers_set,
};

static const StLayers_t ProdStruct = {
    .layers_reset = prod_layers_reset,
    .layers_on_key_press = prod_layers_on_key_press,
    .layers_on_key_release = prod_layers_on_key_release,
    .layers_on_virtual_key = prod_layers_on_virtual_key,
    .layers_get_current = prod_layers_get_current,
    .layers_get_base = prod_layers_get_base,
    .layers_set = prod_layers_set,
};

static StLayers_t ActiveStruct = MockStruct;

// API
void mock_layers_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        ActiveStruct = ProdStruct;
    }
}
StLayers_t* mock_layers_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

LayersInternals_t* mock_layers_get_internals(void) {
    static LayersInternals_t Internals = {
        .layer_state = &layer_state,
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
void layers_reset(void) {
    return ActiveStruct.layers_reset();
}
bool layers_on_key_press(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.layers_on_key_press(row, col, key);
}
bool layers_on_key_release(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.layers_on_key_release(row, col, key);
}
bool layers_on_virtual_key(keymap_entry_t key) {
    return ActiveStruct.layers_on_virtual_key(key);
}
uint8_t layers_get_current(void) {
    return ActiveStruct.layers_get_current();
}
uint8_t layers_get_base(void) {
    return ActiveStruct.layers_get_base();
}
void layers_set(uint8_t layer) {
    return ActiveStruct.layers_set(layer);
}
