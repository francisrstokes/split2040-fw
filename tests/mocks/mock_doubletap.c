#include "mock_doubletap.h"
#include "CppUTestExt/MockSupport_c.h"

#define double_tap_system            prod_double_tap_system

#include "doubletap.c"

#undef double_tap_system

// Mocks
KB_SYSTEM_MOCKS(double_tap)

// Function pointer structs
static const StDoubleTap_t MockStruct = {
    .double_tap_system = mock_double_tap_system,
};

static const StDoubleTap_t ProdStruct = {
    .double_tap_system = prod_double_tap_system,
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
        KB_SYSTEM_INTERNALS_PRIVATE_FN_ASSIGNMENTS(double_tap)
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
const keyboard_system_t* double_tap_system(void) {
    return ActiveStruct.double_tap_system();
}
