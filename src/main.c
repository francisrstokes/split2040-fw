/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"

#include "usb_common.h"
#include "matrix.h"
#include "keyboard.h"
#include "leds.h"

static repeating_timer_t matrix_scan_timer = {0};

static bool matrix_scan_timer_cb(repeating_timer_t *rt) {
    matrix_scan();
    usb_update();
    leds_write();
    return true;
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
    usb_wait_for_device_to_configured();

    // After we're configured, setup a repeating timer for scanning the key matrix
    add_repeating_timer_ms(-MATRIX_SCAN_INTERVAL_MS, matrix_scan_timer_cb, NULL, &matrix_scan_timer);

    while (1) {
        tight_loop_contents();
    }
}
