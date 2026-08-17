#pragma once

#include "keyboard_types.h"

// typedefs
typedef struct keyboard_system_t {
    const char* name;
    void (*init)(void* init_data);
    void (*reset)(void);
    bool (*update)(void);
    bool (*on_press)(uint row, uint col, keymap_entry_t key);
    bool (*on_virtual_press)(keymap_entry_t key);
    bool (*on_release)(uint row, uint col, keymap_entry_t key);
} keyboard_system_t;
