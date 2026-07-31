#ifndef MOCK_KEYBOARD_H
#define MOCK_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

// Include the original header
#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "keyboard.h"

typedef struct StKeyboard_t {
    void (*keyboard_init)(uint8_t* keyboard_hid_report, uint16_t* cc_hid_report, mouse_report_t* mouse_hid_report);
    void (*keyboard_reset)(void);
    bool (*keyboard_send_key)(keymap_entry_t key);
    void (*keyboard_send_modifiers)(uint8_t modifiers);
    void (*keyboard_clear_sent_keys)(void);
    void (*keyboard_post_scan)(void);
    keymap_entry_t (*keyboard_resolve_key)(uint row, uint col);
    keymap_entry_t (*keyboard_resolve_key_on_layer)(uint row, uint col, uint layer);
    uint8_t (*keyboard_get_current_layer)(void);
    void (*keyboard_on_led_status_report)(uint8_t led_status);
    void (*keyboard_set_keymap_ptr)(void* new_keymap);

    // weak functions to be overridden by specific keyboards
    bool (*kbc_on_key_press)(uint row, uint col, keymap_entry_t key);
    bool (*kbc_on_key_release)(uint row, uint col, keymap_entry_t key);
    bool (*kbc_on_virtual_key)(keymap_entry_t key);
    bool (*keyboard_before_send_key)(keymap_entry_t* key);
    void (*keyboard_on_scan_complete)(const uint8_t* hid_report);
} StKeyboard_t;

// Mock API
void mock_keyboard_use_mocks(bool use_mocks);
StKeyboard_t* mock_keyboard_get_fn_ptr_struct(void);

#ifdef __cplusplus
}
#endif

#endif // MOCK_KEYBOARD_H

