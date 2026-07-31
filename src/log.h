/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"

// public functions
void log_str(char* str);
void log_int(uint32_t value);
void log_hex(uint32_t value, bool pad);
void log_ptr(void* ptr);
