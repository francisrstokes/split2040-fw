#pragma once

#include "../split.h"

// defines
#define SPL_I2C_IDLE                    (0)
#define SPL_I2C_WRITE_IN_PROGRESS       (1U << 0U)
#define SPL_I2C_READ_IN_PROGRESS        (1U << 1U)
#define SPL_I2C_OPERATION_IN_PROGRESS   (SPL_I2C_WRITE_IN_PROGRESS | SPL_I2C_READ_IN_PROGRESS)

// typedefs
typedef struct split_i2c_target_t {
    uint32_t state;
    uint8_t address;
    bool read_started;
} split_i2c_target_t;

typedef struct split_i2c_controller_t {
    bool read_ready;
    uint32_t keys[MATRIX_ROWS];
} split_i2c_controller_t;

// public functions
split_impl_t* split_i2c_get_impl(void);
