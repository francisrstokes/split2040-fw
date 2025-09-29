/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "keyboard.h"

// defines
#define NUM_LEDS  (2)

// typedefs
typedef struct leds_state_t {
    uint8_t leds[NUM_LEDS][3];
    uint8_t leds_out[NUM_LEDS][3];
    uint8_t brightness;
    uint8_t mask;
    bool should_transmit;
} leds_state_t;

// public functions
void leds_set_color(uint led_index, uint8_t r, uint8_t g, uint8_t b);
void leds_write(void);
void leds_init(void);
void leds_brightness_up(void);
void leds_brightness_down(void);
void leds_toggle_led_enabled(uint led_index);
