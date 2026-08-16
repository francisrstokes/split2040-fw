#include "mock_macro.h"
#include "CppUTestExt/MockSupport_c.h"


#define macro_system            prod_macro_system
#define macro_any_active        prod_macro_any_active

#include "macro.c"

#undef macro_system
#undef macro_any_active

// Mocks
static bool mock_macro_any_active(void) {
    mock_c()->actualCall("macro_any_active");
    return mock_c()->returnBoolValueOrDefault(false);
}
KB_SYSTEM_MOCKS(macro)

// Function pointer structs
static const StMacro_t MockStruct = {
    .macro_system = mock_macro_system,
    .macro_any_active = mock_macro_any_active,
};

static const StMacro_t ProdStruct = {
    .macro_system = prod_macro_system,
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

        // private functions
        KB_SYSTEM_INTERNALS_PRIVATE_FN_ASSIGNMENTS(macro)
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
const keyboard_system_t* macro_system(void) {
    return ActiveStruct.macro_system();
}

bool macro_any_active(void) {
    return ActiveStruct.macro_any_active();
}
