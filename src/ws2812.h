/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"

// public functions
void ws2812_init(void);
void ws2812_write(uint32_t rgb32);
