/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// typedefs
typedef struct leds_state_t {
    uint8_t leds[LEDS_MAX][3];
    uint8_t leds_out[LEDS_MAX][3];
    uint8_t brightness;
    uint8_t mask;
    bool should_transmit;
} leds_state_t;

// public functions
void leds_set_color(uint led_index, uint8_t r, uint8_t g, uint8_t b);
void leds_set_r(uint led_index, uint8_t value);
void leds_set_g(uint led_index, uint8_t value);
void leds_set_b(uint led_index, uint8_t value);

void leds_write(void);
void leds_init(void);
void leds_brightness_up(void);
void leds_brightness_down(void);
void leds_toggle_led_enabled(uint led_index);
