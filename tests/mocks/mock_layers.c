#include "mock_layers.h"
#include "CppUTestExt/MockSupport_c.h"

#define layers_system            prod_layers_system
#define layers_get_current       prod_layers_get_current
#define layers_get_base          prod_layers_get_base
#define layers_set               prod_layers_set

#include "layers.c"

#undef layers_system
#undef layers_get_current
#undef layers_get_base
#undef layers_set

// Mocks
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
KB_SYSTEM_MOCKS(layers)

// Function pointer structs
static const StLayers_t MockStruct = {
    .layers_system = mock_layers_system,
    .layers_get_current = mock_layers_get_current,
    .layers_get_base = mock_layers_get_base,
    .layers_set = mock_layers_set,
};

static const StLayers_t ProdStruct = {
    .layers_system = prod_layers_system,
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

        // private functions
        KB_SYSTEM_INTERNALS_PRIVATE_FN_ASSIGNMENTS(layers)
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
const keyboard_system_t* layers_system(void) {
    return ActiveStruct.layers_system();
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
