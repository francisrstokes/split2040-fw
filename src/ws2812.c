/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "ws2812.pio.h"
#include "ws2812.h"

// defines
#define WS2812_PIN  (28)

// statics
static PIO pio;
static uint sm;
static uint offset;

// public functions
void ws2812_init(void) {
    pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000);
}

void ws2812_write(uint32_t rgb32) {
    pio_sm_put_blocking(pio, sm, rgb32 << 8);
}
