#pragma once

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Adapted for unit testing
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef unsigned int uint;

typedef uint64_t absolute_time_t;

static inline uint64_t to_us_since_boot(absolute_time_t t) {
    return t;
}

static inline void update_us_since_boot(absolute_time_t *t, uint64_t us_since_boot) {
    *t = us_since_boot;
}

static inline absolute_time_t from_us_since_boot(uint64_t us_since_boot) {
    absolute_time_t t;
    update_us_since_boot(&t, us_since_boot);
    return t;
}

#define ABSOLUTE_TIME_INITIALIZED_VAR(name, value) name = {value}

typedef struct {
    int16_t year;    ///< 0..4095
    int8_t month;    ///< 1..12, 1 is January
    int8_t day;      ///< 1..28,29,30,31 depending on month
    int8_t dotw;     ///< 0..6, 0 is Sunday
    int8_t hour;     ///< 0..23
    int8_t min;      ///< 0..59
    int8_t sec;      ///< 0..59
} datetime_t;

#define bool_to_bit(x) ((uint)!!(x))
