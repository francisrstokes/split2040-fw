#ifndef MOCK_MACRO_H
#define MOCK_MACRO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "machines/machine.h"
#include "macro.h"

typedef struct StMacro_t {
    void (*macro_init)(macro_t* macro_table);
    void (*macro_reset)(void);
    bool (*macro_on_key_press)(uint row, uint col, keymap_entry_t key);
    bool (*macro_on_key_release)(uint row, uint col, keymap_entry_t key);
    bool (*macro_on_virtual_key)(keymap_entry_t key);
    bool (*macro_update)(void);
    bool (*macro_any_active)(void);
} StMacro_t;

typedef struct MacroInternals_t {
    volatile macro_t** macros;
    const uint8_t (*ascii_to_hid_kc)[128][2];
    bool* any_macro_active;
} MacroInternals_t;

// Mock API
void mock_macro_use_mocks(bool use_mocks);
StMacro_t* mock_macro_get_fn_ptr_struct(void);
MacroInternals_t* mock_macro_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
