/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"

// defines
#define WHITE       255, 255, 255
#define CYAN        0,   255, 255
#define ORANGE      255, 200, 0
#define MAGENTA     255, 0,   255
#define AQUAMARINE  0,   255, 127

// typedefs

// public functions
void color_rgb2hsl(const uint8_t* rgb, uint8_t* hsl);
void color_hsl2rgb(const uint8_t* hsl, uint8_t* rgb);
