// mock_log.c
#include "mock_log.h"
#include "CppUTestExt/MockSupport_c.h"

// #define log_str    prod_log_str
// #define log_int    prod_log_int
// #define log_hex    prod_log_hex
// #define log_ptr    prod_log_ptr

// #include "log.c"

// #undef log_str
// #undef log_int
// #undef log_hex
// #undef log_ptr

static bool expectations_enabled = true;

// Mocks
static void mock_log_str(char* str) {
    if (expectations_enabled) {
        mock_c()->actualCall("log_str")
        ->withStringParameters("str", str);
    }
}
static void mock_log_int(uint32_t value) {
    if (expectations_enabled) {
        mock_c()->actualCall("log_int")
        ->withUnsignedIntParameters("value", value);
    }
}
static void mock_log_hex(uint32_t value, bool pad) {
    if (expectations_enabled) {
        mock_c()->actualCall("log_hex")
        ->withUnsignedIntParameters("value", value)
        ->withBoolParameters("pad", pad);
    }
}
static void mock_log_ptr(void* ptr) {
    if (expectations_enabled) {
        mock_c()->actualCall("log_ptr")
        ->withPointerParameters("ptr", ptr);
    }
}

// Function pointer structs
static const StLog_t MockStruct = {
    .log_str = mock_log_str,
    .log_int = mock_log_int,
    .log_hex = mock_log_hex,
    .log_ptr = mock_log_ptr,
};

// static const StLog_t ProdStruct = {
//     .log_str = prod_log_str,
//     .log_int = prod_log_int,
//     .log_hex = prod_log_hex,
//     .log_ptr = prod_log_ptr,
// };

static StLog_t ActiveStruct = MockStruct;

// API
void mock_log_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        // ActiveStruct = ProdStruct;
    }
}
StLog_t* mock_log_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

LogInternals_t* mock_log_get_internals(void) {
    static LogInternals_t Internals = {0};

    return &Internals;
}

void mock_log_expect_calls(bool should_expect) {
    expectations_enabled = should_expect;
}

// Originally named functions that can be diverted to function pointers
void log_str(char* str) {
    return ActiveStruct.log_str(str);
}
void log_int(uint32_t value) {
    return ActiveStruct.log_int(value);
}
void log_hex(uint32_t value, bool pad) {
    return ActiveStruct.log_hex(value, pad);
}
void log_ptr(void* ptr) {
    return ActiveStruct.log_ptr(ptr);
}
