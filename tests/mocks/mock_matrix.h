#ifndef MOCK_MATRIX_H
#define MOCK_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define MATRIX_ROWS                 (4)
#define MATRIX_COLS                 (12)
#define MATRIX_SETTLE_ITERATIONS    (50)

#include "matrix.h"

typedef struct StMatrix_t {
    void (*matrix_init)(void);
    void (*matrix_reset)(void);
    void (*matrix_scan)(void);
    bool (*matrix_key_pressed)(uint32_t row, uint32_t col, bool also_when_handled);
    bool (*matrix_key_pressed_this_scan)(uint32_t row, uint32_t col);
    bool (*matrix_key_released_this_scan)(uint32_t row, uint32_t col);
    void (*matrix_suppress_held_until_release)(void);
    void (*matrix_suppress_key_until_release)(uint32_t row, uint32_t col);
    void (*matrix_mark_key_as_handled)(uint32_t row, uint32_t col);
    void (*matrix_mark_key_as_unhandled)(uint32_t row, uint32_t col);
    const uint32_t* (*matrix_get_pressed_bitmap)(void);
    const uint32_t* (*matrix_get_handled_bitmap)(void);
    const uint32_t* (*matrix_get_pressed_this_scan_bitmap)(void);
    const uint32_t* (*matrix_get_released_this_scan_bitmap)(void);
    const uint (*matrix_get_col_gpio)(uint col);
    const uint (*matrix_get_row_gpio)(uint row);
} StMatrix_t;

typedef struct MatrixInternals_t {
    uint32_t (*prev_pressed_bitmap)[MATRIX_ROWS];
    uint32_t (*pressed_bitmap)[MATRIX_ROWS];
    uint32_t (*handled_bitmap)[MATRIX_ROWS];
    uint32_t (*pressed_this_scan_bitmap)[MATRIX_ROWS];
    uint32_t (*released_this_scan_bitmap)[MATRIX_ROWS];
    uint32_t (*suppressed_until_release)[MATRIX_ROWS];
} MatrixInternals_t;

// Mock API
void mock_matrix_use_mocks(bool use_mocks);
StMatrix_t* mock_matrix_get_fn_ptr_struct(void);
MatrixInternals_t* mock_matrix_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
