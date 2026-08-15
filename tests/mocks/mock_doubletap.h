#ifndef MOCK_DOUBLETAP_H
#define MOCK_DOUBLETAP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "machines/machine.h"
#include "doubletap.h"

typedef struct StDoubleTap_t {
    void (*double_tap_init)(void);
    void (*double_tap_reset)(void);
    bool (*double_tap_update)(void);
    bool (*double_tap_on_key_release)(uint row, uint col, keymap_entry_t key);
    bool (*double_tap_on_key_press)(uint row, uint col, keymap_entry_t key);
} StDoubleTap_t;

typedef struct DoubleTapInternals_t {
    double_tap_state_t* double_taps;

    // Private functions
    bool (*double_tap_is_matching_key)(double_tap_data_t* dt, keymap_entry_t key);
    ll_node_t* (*double_tap_find_active)(keymap_entry_t key);
} DoubleTapInternals_t;

// Mock API
void mock_double_tap_use_mocks(bool use_mocks);
StDoubleTap_t* mock_double_tap_get_fn_ptr_struct(void);
DoubleTapInternals_t* mock_double_tap_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
