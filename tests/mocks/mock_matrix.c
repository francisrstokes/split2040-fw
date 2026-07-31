#include "mock_matrix.h"
#include "CppUTestExt/MockSupport_c.h"

// #define matrix_init                             prod_matrix_init
// #define matrix_reset                            prod_matrix_reset
// #define matrix_scan                             prod_matrix_scan
// #define matrix_key_pressed                      prod_matrix_key_pressed
// #define matrix_key_pressed_this_scan            prod_matrix_key_pressed_this_scan
// #define matrix_key_released_this_scan           prod_matrix_key_released_this_scan
// #define matrix_suppress_held_until_release      prod_matrix_suppress_held_until_release
// #define matrix_suppress_key_until_release       prod_matrix_suppress_key_until_release
// #define matrix_mark_key_as_handled              prod_matrix_mark_key_as_handled
// #define matrix_mark_key_as_unhandled            prod_matrix_mark_key_as_unhandled
// #define matrix_get_pressed_bitmap               prod_matrix_get_pressed_bitmap
// #define matrix_get_handled_bitmap               prod_matrix_get_handled_bitmap
// #define matrix_get_pressed_this_scan_bitmap     prod_matrix_get_pressed_this_scan_bitmap
// #define matrix_get_released_this_scan_bitmap    prod_matrix_get_released_this_scan_bitmap
// #define matrix_get_col_gpio                     prod_matrix_get_col_gpio
// #define matrix_get_row_gpio                     prod_matrix_get_row_gpio

// #include "matrix.c"

// #undef matrix_init
// #undef matrix_reset
// #undef matrix_scan
// #undef matrix_key_pressed
// #undef matrix_key_pressed_this_scan
// #undef matrix_key_released_this_scan
// #undef matrix_suppress_held_until_release
// #undef matrix_suppress_key_until_release
// #undef matrix_mark_key_as_handled
// #undef matrix_mark_key_as_unhandled
// #undef matrix_get_pressed_bitmap
// #undef matrix_get_handled_bitmap
// #undef matrix_get_pressed_this_scan_bitmap
// #undef matrix_get_released_this_scan_bitmap
// #undef matrix_get_col_gpio
// #undef matrix_get_row_gpio

// Mocks
static void mock_matrix_init(void) {
    mock_c()->actualCall("matrix_init");
}
static void mock_matrix_reset(void) {
    mock_c()->actualCall("matrix_reset");
}
static void mock_matrix_scan(void) {
    mock_c()->actualCall("matrix_scan");
}
static bool mock_matrix_key_pressed(uint32_t row, uint32_t col, bool also_when_handled) {
    mock_c()->actualCall("matrix_key_pressed")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col)
    ->withBoolParameters("also_when_handled", also_when_handled);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_matrix_key_pressed_this_scan(uint32_t row, uint32_t col) {
    mock_c()->actualCall("matrix_key_pressed_this_scan")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_matrix_key_released_this_scan(uint32_t row, uint32_t col) {
    mock_c()->actualCall("matrix_key_released_this_scan")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col);
    return mock_c()->returnBoolValueOrDefault(false);
}
static void mock_matrix_suppress_held_until_release(void) {
    mock_c()->actualCall("matrix_suppress_held_until_release");
}
static void mock_matrix_suppress_key_until_release(uint32_t row, uint32_t col) {
    mock_c()->actualCall("matrix_suppress_key_until_release")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col);
}
static void mock_matrix_mark_key_as_handled(uint32_t row, uint32_t col) {
    mock_c()->actualCall("matrix_mark_key_as_handled")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col);
}
static void mock_matrix_mark_key_as_unhandled(uint32_t row, uint32_t col) {
    mock_c()->actualCall("matrix_mark_key_as_unhandled")
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("col", col);
}
static const uint32_t* mock_matrix_get_pressed_bitmap(void) {
    mock_c()->actualCall("matrix_get_pressed_bitmap");
    return mock_c()->returnConstPointerValueOrDefault(NULL);
}
static const uint32_t* mock_matrix_get_handled_bitmap(void) {
    mock_c()->actualCall("matrix_get_handled_bitmap");
    return mock_c()->returnConstPointerValueOrDefault(NULL);
}
static const uint32_t* mock_matrix_get_pressed_this_scan_bitmap(void) {
    mock_c()->actualCall("matrix_get_pressed_this_scan_bitmap");
    return mock_c()->returnConstPointerValueOrDefault(NULL);
}
static const uint32_t* mock_matrix_get_released_this_scan_bitmap(void) {
    mock_c()->actualCall("matrix_get_released_this_scan_bitmap");
    return mock_c()->returnConstPointerValueOrDefault(NULL);
}
static const uint mock_matrix_get_col_gpio(uint col) {
    mock_c()->actualCall("matrix_get_col_gpio");
    return mock_c()->returnUnsignedIntValueOrDefault(0);
}
static const uint mock_matrix_get_row_gpio(uint row) {
    mock_c()->actualCall("matrix_get_row_gpio");
    return mock_c()->returnUnsignedIntValueOrDefault(0);
}

// Function pointer structs
static const StMatrix_t MockStruct = {
    .matrix_init = mock_matrix_init,
    .matrix_reset = mock_matrix_reset,
    .matrix_scan = mock_matrix_scan,
    .matrix_key_pressed = mock_matrix_key_pressed,
    .matrix_key_pressed_this_scan = mock_matrix_key_pressed_this_scan,
    .matrix_key_released_this_scan = mock_matrix_key_released_this_scan,
    .matrix_suppress_held_until_release = mock_matrix_suppress_held_until_release,
    .matrix_suppress_key_until_release = mock_matrix_suppress_key_until_release,
    .matrix_mark_key_as_handled = mock_matrix_mark_key_as_handled,
    .matrix_mark_key_as_unhandled = mock_matrix_mark_key_as_unhandled,
    .matrix_get_pressed_bitmap = mock_matrix_get_pressed_bitmap,
    .matrix_get_handled_bitmap = mock_matrix_get_handled_bitmap,
    .matrix_get_pressed_this_scan_bitmap = mock_matrix_get_pressed_this_scan_bitmap,
    .matrix_get_released_this_scan_bitmap = mock_matrix_get_released_this_scan_bitmap,
    .matrix_get_col_gpio = mock_matrix_get_col_gpio,
    .matrix_get_row_gpio = mock_matrix_get_row_gpio,
};

// static const StMatrix_t ProdStruct = {
//     .matrix_init = prod_matrix_init,
//     .matrix_reset = prod_matrix_reset,
//     .matrix_scan = prod_matrix_scan,
//     .matrix_key_pressed = prod_matrix_key_pressed,
//     .matrix_key_pressed_this_scan = prod_matrix_key_pressed_this_scan,
//     .matrix_key_released_this_scan = prod_matrix_key_released_this_scan,
//     .matrix_suppress_held_until_release = prod_matrix_suppress_held_until_release,
//     .matrix_suppress_key_until_release = prod_matrix_suppress_key_until_release,
//     .matrix_mark_key_as_handled = prod_matrix_mark_key_as_handled,
//     .matrix_mark_key_as_unhandled = prod_matrix_mark_key_as_unhandled,
//     .matrix_get_pressed_bitmap = prod_matrix_get_pressed_bitmap,
//     .matrix_get_handled_bitmap = prod_matrix_get_handled_bitmap,
//     .matrix_get_pressed_this_scan_bitmap = prod_matrix_get_pressed_this_scan_bitmap,
//     .matrix_get_released_this_scan_bitmap = prod_matrix_get_released_this_scan_bitmap,
//     .matrix_get_col_gpio = prod_matrix_get_col_gpio,
//     .matrix_get_row_gpio = prod_matrix_get_row_gpio,
// };

static StMatrix_t ActiveStruct = MockStruct;

// API
void mock_matrix_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        // ActiveStruct = ProdStruct;
    }
}
StMatrix_t* mock_matrix_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

// Originally named functions that can be diverted to function pointers
void matrix_init(void) {
    return ActiveStruct.matrix_init();
}
void matrix_reset(void) {
    return ActiveStruct.matrix_reset();
}
void matrix_scan(void) {
    return ActiveStruct.matrix_scan();
}
bool matrix_key_pressed(uint32_t row, uint32_t col, bool also_when_handled) {
    return ActiveStruct.matrix_key_pressed(row, col, also_when_handled);
}
bool matrix_key_pressed_this_scan(uint32_t row, uint32_t col) {
    return ActiveStruct.matrix_key_pressed_this_scan(row, col);
}
bool matrix_key_released_this_scan(uint32_t row, uint32_t col) {
    return ActiveStruct.matrix_key_released_this_scan(row, col);
}
void matrix_suppress_held_until_release(void) {
    return ActiveStruct.matrix_suppress_held_until_release();
}
void matrix_suppress_key_until_release(uint32_t row, uint32_t col) {
    return ActiveStruct.matrix_suppress_key_until_release(row, col);
}
void matrix_mark_key_as_handled(uint32_t row, uint32_t col) {
    return ActiveStruct.matrix_mark_key_as_handled(row, col);
}
void matrix_mark_key_as_unhandled(uint32_t row, uint32_t col) {
    return ActiveStruct.matrix_mark_key_as_unhandled(row, col);
}
const uint32_t* matrix_get_pressed_bitmap(void) {
    return ActiveStruct.matrix_get_pressed_bitmap();
}
const uint32_t* matrix_get_handled_bitmap(void) {
    return ActiveStruct.matrix_get_handled_bitmap();
}
const uint32_t* matrix_get_pressed_this_scan_bitmap(void) {
    return ActiveStruct.matrix_get_pressed_this_scan_bitmap();
}
const uint32_t* matrix_get_released_this_scan_bitmap(void) {
    return ActiveStruct.matrix_get_released_this_scan_bitmap();
}
const uint matrix_get_col_gpio(uint col) {
    return ActiveStruct.matrix_get_col_gpio(col);
}
const uint matrix_get_row_gpio(uint row) {
    return ActiveStruct.matrix_get_row_gpio(row);
}
