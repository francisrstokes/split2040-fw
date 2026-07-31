#include "mock_keyboard.h"
#include "CppUTestExt/MockSupport_c.h"
#include <stdio.h>

// #define keyboard_init                 prod_keyboard_init
// #define keyboard_reset                prod_keyboard_reset
// #define keyboard_send_key             prod_keyboard_send_key
// #define keyboard_send_modifiers       prod_keyboard_send_modifiers
// #define keyboard_clear_sent_keys      prod_keyboard_clear_sent_keys
// #define keyboard_post_scan            prod_keyboard_post_scan
// #define keyboard_resolve_key          prod_keyboard_resolve_key
// #define keyboard_resolve_key_on_layer prod_keyboard_resolve_key_on_layer
// #define keyboard_get_current_layer    prod_keyboard_get_current_layer
// #define keyboard_on_led_status_report prod_keyboard_on_led_status_report
// #define keyboard_set_keymap_ptr       prod_keyboard_set_keymap_ptr
// #define kbc_on_key_press              prod_kbc_on_key_press
// #define kbc_on_key_release            prod_kbc_on_key_release
// #define kbc_on_virtual_key            prod_kbc_on_virtual_key
// #define keyboard_before_send_key      prod_keyboard_before_send_key
// #define keyboard_on_scan_complete     prod_keyboard_on_scan_complete

// #include "keyboard.c"

// #undef keyboard_init
// #undef keyboard_reset
// #undef keyboard_send_key
// #undef keyboard_send_modifiers
// #undef keyboard_clear_sent_keys
// #undef keyboard_post_scan
// #undef keyboard_resolve_key
// #undef keyboard_resolve_key_on_layer
// #undef keyboard_get_current_layer
// #undef keyboard_on_led_status_report
// #undef keyboard_set_keymap_ptr
// #undef kbc_on_key_press
// #undef kbc_on_key_release
// #undef kbc_on_virtual_key
// #undef keyboard_before_send_key
// #undef keyboard_on_scan_complete

// Mocks
static void mock_keyboard_init(uint8_t* keyboard_hid_report, uint16_t* cc_hid_report, mouse_report_t* mouse_hid_report) {
    mock_c()->actualCall("keyboard_init")
    ->withPointerParameters("keyboard_hid_report", (void*)keyboard_hid_report)
    ->withPointerParameters("cc_hid_report", (void*)cc_hid_report)
    ->withPointerParameters("mouse_hid_report", (void*)mouse_hid_report);
}
static void mock_keyboard_reset(void) {
    mock_c()->actualCall("keyboard_reset");
}
static bool mock_keyboard_send_key(keymap_entry_t key) {
    // printf("send_key: %02x\n", key);
    mock_c()->actualCall("keyboard_send_key")->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(true);
}
static void mock_keyboard_send_modifiers(uint8_t modifiers) {
    mock_c()->actualCall("keyboard_send_modifiers")->withUnsignedIntParameters("modifiers", modifiers);
}
static void mock_keyboard_clear_sent_keys(void) {
    mock_c()->actualCall("keyboard_clear_sent_keys");
}
static void mock_keyboard_post_scan(void) {
    mock_c()->actualCall("keyboard_post_scan");
}
static keymap_entry_t mock_keyboard_resolve_key(uint row, uint col) {
    mock_c()->actualCall("keyboard_resolve_key")
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("row", row);
    return (keymap_entry_t)(mock_c()->returnUnsignedIntValueOrDefault(0));
}
static keymap_entry_t mock_keyboard_resolve_key_on_layer(uint row, uint col, uint layer) {
    mock_c()->actualCall("keyboard_resolve_key_on_layer")
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("layer", layer);
    return (keymap_entry_t)(mock_c()->returnUnsignedIntValueOrDefault(0));
}
static uint8_t mock_keyboard_get_current_layer(void) {
    mock_c()->actualCall("keyboard_get_current_layer");
    return mock_c()->returnUnsignedIntValueOrDefault(0);
}
static void mock_keyboard_on_led_status_report(uint8_t led_status) {
    mock_c()->actualCall("keyboard_on_led_status_report")
    ->withUnsignedIntParameters("led_status", led_status);
}
static void mock_keyboard_set_keymap_ptr(void* new_keymap) {
    mock_c()->actualCall("keyboard_set_keymap_ptr")
    ->withPointerParameters("new_keymap", (void*)new_keymap);
}
bool mock_kbc_on_key_press(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("kbc_on_key_press")
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
bool mock_kbc_on_key_release(uint row, uint col, keymap_entry_t key) {
    mock_c()->actualCall("kbc_on_key_release")
    ->withUnsignedIntParameters("col", col)
    ->withUnsignedIntParameters("row", row)
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
bool mock_kbc_on_virtual_key(keymap_entry_t key) {
    mock_c()->actualCall("kbc_on_virtual_key")
    ->withUnsignedIntParameters("key", key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static bool mock_keyboard_before_send_key(keymap_entry_t* key) {
    mock_c()->actualCall("keyboard_before_send_key")
    ->withPointerParameters("key", (void*)key);
    return mock_c()->returnBoolValueOrDefault(false);
}
static void mock_keyboard_on_scan_complete(const uint8_t* hid_report) {
     mock_c()->actualCall("keyboard_before_send_key")
    ->withConstPointerParameters("hid_report", (const void*)hid_report);
}

// Function pointer structs
static const StKeyboard_t MockStruct = {
    .keyboard_init = mock_keyboard_init,
    .keyboard_reset = mock_keyboard_reset,
    .keyboard_send_key = mock_keyboard_send_key,
    .keyboard_send_modifiers = mock_keyboard_send_modifiers,
    .keyboard_clear_sent_keys = mock_keyboard_clear_sent_keys,
    .keyboard_post_scan = mock_keyboard_post_scan,
    .keyboard_resolve_key = mock_keyboard_resolve_key,
    .keyboard_resolve_key_on_layer = mock_keyboard_resolve_key_on_layer,
    .keyboard_get_current_layer = mock_keyboard_get_current_layer,
    .keyboard_on_led_status_report = mock_keyboard_on_led_status_report,
    .keyboard_set_keymap_ptr = mock_keyboard_set_keymap_ptr,

    // weak functions to be overridden by specific keyboards
    .kbc_on_key_press = mock_kbc_on_key_press,
    .kbc_on_key_release = mock_kbc_on_key_release,
    .kbc_on_virtual_key = mock_kbc_on_virtual_key,
    .keyboard_before_send_key = mock_keyboard_before_send_key,
    .keyboard_on_scan_complete = mock_keyboard_on_scan_complete,
};

// static const StKeyboard_t ProdStruct = {
//     .keyboard_init = prod_keyboard_init,
//     .keyboard_reset = prod_keyboard_reset,
//     .keyboard_send_key = prod_keyboard_send_key,
//     .keyboard_send_modifiers = prod_keyboard_send_modifiers,
//     .keyboard_clear_sent_keys = prod_keyboard_clear_sent_keys,
//     .keyboard_post_scan = prod_keyboard_post_scan,
//     .keyboard_resolve_key = prod_keyboard_resolve_key,
//     .keyboard_resolve_key_on_layer = prod_keyboard_resolve_key_on_layer,
//     .keyboard_get_current_layer = prod_keyboard_get_current_layer,
//     .keyboard_on_led_status_report = prod_keyboard_on_led_status_report,
//     .keyboard_set_keymap_ptr = prod_keyboard_set_keymap_ptr,

//     // weak functions to be overridden by specific keyboards
//     .kbc_on_key_press = prod_kbc_on_key_press,
//     .kbc_on_key_release = prod_kbc_on_key_release,
//     .kbc_on_virtual_key = prod_kbc_on_virtual_key,
//     .keyboard_before_send_key = prod_keyboard_before_send_key,
//     .keyboard_on_scan_complete = prod_keyboard_on_scan_complete,
// };

static StKeyboard_t ActiveStruct = MockStruct;

// API
void mock_keyboard_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        // ActiveStruct = ProdStruct;
    }
}
StKeyboard_t* mock_keyboard_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

// Originally named functions that can be diverted to function pointers
void keyboard_init(uint8_t* keyboard_hid_report, uint16_t* cc_hid_report, mouse_report_t* mouse_hid_report) {
    return ActiveStruct.keyboard_init(keyboard_hid_report, cc_hid_report, mouse_hid_report);
}
void keyboard_reset(void) {
    return ActiveStruct.keyboard_reset();
}
bool keyboard_send_key(keymap_entry_t key) {
    return ActiveStruct.keyboard_send_key(key);
}
void keyboard_send_modifiers(uint8_t modifiers) {
    return ActiveStruct.keyboard_send_modifiers(modifiers);
}
void keyboard_clear_sent_keys(void) {
    return ActiveStruct.keyboard_clear_sent_keys();
}
void keyboard_post_scan(void) {
    return ActiveStruct.keyboard_post_scan();
}
keymap_entry_t keyboard_resolve_key(uint row, uint col) {
    return ActiveStruct.keyboard_resolve_key(row, col);
}
keymap_entry_t keyboard_resolve_key_on_layer(uint row, uint col, uint layer) {
    return ActiveStruct.keyboard_resolve_key_on_layer(row, col, layer);
}
uint8_t keyboard_get_current_layer(void) {
    return ActiveStruct.keyboard_get_current_layer();
}
void keyboard_on_led_status_report(uint8_t led_status) {
    return ActiveStruct.keyboard_on_led_status_report(led_status);
}
void keyboard_set_keymap_ptr(void* new_keymap) {
    return ActiveStruct.keyboard_set_keymap_ptr(new_keymap);
}
bool kbc_on_key_press(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.kbc_on_key_press(row, col, key);
}
bool kbc_on_key_release(uint row, uint col, keymap_entry_t key) {
    return ActiveStruct.kbc_on_key_release(row, col, key);
}
bool kbc_on_virtual_key(keymap_entry_t key) {
    return ActiveStruct.kbc_on_virtual_key(key);
}
bool keyboard_before_send_key(keymap_entry_t* key) {
    return ActiveStruct.keyboard_before_send_key(key);
}
void keyboard_on_scan_complete(const uint8_t* hid_report) {
    return ActiveStruct.keyboard_on_scan_complete(hid_report);
}
