/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

// defines
#define WHITE   255, 255, 255
#define CYAN    0,   255, 255
#define ORANGE  255, 200, 0
#define MAGENTA 255, 0,   255

// public functions
void ws2812_init(void);
void ws2812_set_color(uint led_index, uint8_t r, uint8_t g, uint8_t b);
void ws2812_write(void);
