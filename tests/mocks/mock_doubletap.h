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
#include "mock_keyboard_system.h"

typedef struct StDoubleTap_t {
    const keyboard_system_t* (*double_tap_system)(void);
} StDoubleTap_t;

typedef struct DoubleTapInternals_t {
    double_tap_state_t* double_taps;

    // Private functions
    bool (*double_tap_is_matching_key)(double_tap_data_t* dt, keymap_entry_t key);
    ll_node_t* (*double_tap_find_active)(keymap_entry_t key);
    KB_SYSTEM_INTERNALS_PRIVATE_FNS(double_tap)
} DoubleTapInternals_t;

// Mock API
void mock_double_tap_use_mocks(bool use_mocks);
StDoubleTap_t* mock_double_tap_get_fn_ptr_struct(void);
DoubleTapInternals_t* mock_double_tap_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
