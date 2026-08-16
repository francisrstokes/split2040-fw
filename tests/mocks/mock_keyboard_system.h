#ifndef MOCK_KEYBOARD_SYSTEM_H
#define MOCK_KEYBOARD_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "keyboard_system.h"

#define KB_SYSTEM_MOCK_INIT(system_name) \
static void mock_ ## system_name ## _init(void* init_data) { \
    mock_c()->actualCall(#system_name "_init") \
    ->withPointerParameters("init_data", init_data); \
}

#define KB_SYSTEM_MOCK_RESET(system_name) \
static void mock_ ## system_name ## _reset(void) { \
    mock_c()->actualCall(#system_name "_reset"); \
}

#define KB_SYSTEM_MOCK_UPDATE(system_name) \
static bool mock_ ## system_name ## _update(void) { \
    mock_c()->actualCall(#system_name "_update"); \
    return mock_c()->returnBoolValueOrDefault(false); \
}

#define KB_SYSTEM_MOCK_ON_PRESS(system_name) \
static bool mock_ ## system_name ## _on_press(uint row, uint col, keymap_entry_t key) { \
    mock_c()->actualCall(#system_name "_on_press") \
    ->withUnsignedIntParameters("row", row) \
    ->withUnsignedIntParameters("col", col) \
    ->withUnsignedIntParameters("key", key); \
    return mock_c()->returnBoolValueOrDefault(false); \
}

#define KB_SYSTEM_MOCK_ON_VIRTUAL_PRESS(system_name) \
static bool mock_ ## system_name ## _on_virtual_press(keymap_entry_t key) { \
    mock_c()->actualCall(#system_name "_on_virtual_press") \
    ->withUnsignedIntParameters("key", key); \
    return mock_c()->returnBoolValueOrDefault(false); \
}

#define KB_SYSTEM_MOCK_ON_RELEASE(system_name) \
static bool mock_ ## system_name ## _on_release(uint row, uint col, keymap_entry_t key) { \
    mock_c()->actualCall(#system_name "_on_release") \
    ->withUnsignedIntParameters("row", row) \
    ->withUnsignedIntParameters("col", col) \
    ->withUnsignedIntParameters("key", key); \
    return mock_c()->returnBoolValueOrDefault(false); \
}

#define KB_SYSTEM_FN_PTR(system_name) const keyboard_system_t* (*system_name ## _system)(void);

#define KB_SYSTEM_INTERNALS_PRIVATE_FNS(system_name) \
    void (*system_name ## _init)(void* init_data); \
    void (*system_name ## _reset)(void); \
    bool (*system_name ## _update)(void); \
    bool (*system_name ## _on_release)(uint row, uint col, keymap_entry_t key); \
    bool (*system_name ## _on_press)(uint row, uint col, keymap_entry_t key); \
    bool (*system_name ## _on_virtual_press)(keymap_entry_t key);

#define KB_SYSTEM_INTERNALS_PRIVATE_FN_ASSIGNMENTS(system_name) \
    .system_name ## _init = system_name ## _init, \
    .system_name ## _reset = system_name ## _reset, \
    .system_name ## _update = system_name ## _update, \
    .system_name ## _on_release = system_name ## _on_release, \
    .system_name ## _on_press = system_name ## _on_press, \
    .system_name ## _on_virtual_press = system_name ## _on_virtual_press,

#define KB_SYSTEM_MOCK_SYSTEM_FN(system_name) \
static const keyboard_system_t* mock_ ## system_name ## _system(void) { \
    static keyboard_system_t system = { \
        .name = #system_name, \
        .init = mock_ ## system_name ## _init, \
        .reset = mock_ ## system_name ## _reset, \
        .update = mock_ ## system_name ## _update, \
        .on_press = mock_ ## system_name ## _on_press, \
        .on_virtual_press = mock_ ## system_name ## _on_virtual_press, \
        .on_release = mock_ ## system_name ## _on_release, \
    }; \
    mock_c()->actualCall(#system_name "_system"); \
    return &system; \
}

#define KB_SYSTEM_MOCKS(system_name) \
KB_SYSTEM_MOCK_INIT(system_name) \
KB_SYSTEM_MOCK_RESET(system_name) \
KB_SYSTEM_MOCK_UPDATE(system_name) \
KB_SYSTEM_MOCK_ON_PRESS(system_name) \
KB_SYSTEM_MOCK_ON_VIRTUAL_PRESS(system_name) \
KB_SYSTEM_MOCK_ON_RELEASE(system_name) \
KB_SYSTEM_MOCK_SYSTEM_FN(system_name)

#ifdef __cplusplus
}
#endif

#endif
