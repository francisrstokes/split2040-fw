/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"


#define NUM_PIXELS      (4)
#define WS2812_PIN      (6)

// statics
static PIO pio;
static uint sm;
static uint offset;

static uint brightness_div = 4;
static bool change_since_last_write = true;
static uint32_t leds[NUM_PIXELS] = { 0xffffff, 0 };

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void ws2812_init(void) {
    pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000);
}

void ws2812_set_color(uint led_index, uint8_t r, uint8_t g, uint8_t b) {
    if (led_index >= NUM_PIXELS) return;
    leds[led_index] = urgb_u32(r, g, b);
    change_since_last_write = true;
}

void ws2812_write(void) {
    if (change_since_last_write) {
        for (uint i = 0; i < NUM_PIXELS; i++) {
            put_pixel(pio, sm, leds[NUM_PIXELS - i - 1] / brightness_div);
        }
    }
    change_since_last_write = false;
}
