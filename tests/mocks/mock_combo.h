#ifndef MOCK_COMBO_H
#define MOCK_COMBO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "machines/machine.h"
#include "combo.h"
#include "mock_keyboard_system.h"

typedef struct StCombo_t {
    KB_SYSTEM_FN_PTR(combo);
} StCombo_t;

typedef struct ComboInternals_t {
    combo_t** combos;

    // Private functions
    int (*combo_get_key_index)(uint combo_index, keymap_entry_t key);
    int (*combo_find_next_with_key)(uint start_index, keymap_entry_t key);
    void (*combo_update_key_in_active)(uint combo_index, uint key_index, uint row, uint col);
    void (*combo_start)(uint combo_index, uint key_index);
    bool (*combo_is_complete)(uint combo_index);
    int (*combo_get_single_pressed_index)(uint combo_index);
    void (*combo_mark_keys_as_handled)(uint combo_index);
    void (*combo_deactivate_unfinished_overlapping_combos)(uint combo_index);
    KB_SYSTEM_INTERNALS_PRIVATE_FNS(combo)
} ComboInternals_t;

// Mock API
void mock_combo_use_mocks(bool use_mocks);
StCombo_t* mock_combo_get_fn_ptr_struct(void);
ComboInternals_t* mock_combo_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
