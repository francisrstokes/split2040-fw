/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/sync.h"

#include "usb_common.h"
#include "matrix.h"
#include "keyboard.h"
#include "kb_config.h"
#include "leds.h"

static repeating_timer_t update_timer = {0};
static bool update_time_elapsed = false;

static bool update_timer_callback(repeating_timer_t *rt) {
    update_time_elapsed = true;
    return true;
}

static void run_keyboard_update(void) {
    matrix_scan();
    usb_update();
    leds_write();
}

int main(void) {
    leds_init();
    matrix_init();
    keyboard_init(
        usb_get_kb_hid_descriptor_ptr(),
        usb_get_cc_hid_descriptor_ptr(),
        usb_get_mouse_hid_descriptor_ptr()
    );

    usb_device_init();
    kb_config_init();
    usb_wait_for_device_to_configured();

    // After we're configured, setup a repeating timer for scanning the key matrix
    add_repeating_timer_ms(-MATRIX_SCAN_INTERVAL_MS, update_timer_callback, NULL, &update_timer);

    while (1) {
        if (update_time_elapsed) {
            update_time_elapsed = false;
            run_keyboard_update();
        }
        __wfi();
    }
}
