#include "CppUTest/TestHarness.h"
#include "CppUTest/PlatformSpecificFunctions.h"
#include "CppUTestExt/MockSupport.h"

#include <vector>
#include <unordered_map>
#include <algorithm>

#include "mock_matrix.h"
#include "hardware/gpio.h"

TEST_GROUP(matrix) {

    MatrixInternals_t* internals = mock_matrix_get_internals();

    void setup() {
        mock_matrix_use_mocks(false);

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_matrix_use_mocks(true);

        reset_environment();
    }

    void reset_environment(void) {
        PlatformSpecificMemset(*internals->handled_bitmap, 0, sizeof(*internals->handled_bitmap));
        PlatformSpecificMemset(*internals->pressed_bitmap, 0, sizeof(*internals->pressed_bitmap));
        PlatformSpecificMemset(*internals->pressed_this_scan_bitmap, 0, sizeof(*internals->pressed_this_scan_bitmap));
        PlatformSpecificMemset(*internals->prev_pressed_bitmap, 0, sizeof(*internals->prev_pressed_bitmap));
        PlatformSpecificMemset(*internals->released_this_scan_bitmap, 0, sizeof(*internals->released_this_scan_bitmap));
        PlatformSpecificMemset(*internals->suppressed_until_release, 0, sizeof(*internals->suppressed_until_release));
    }
};

TEST(matrix, matrix_init_sets_up_rows_and_cols)
{
    // Setup
    // (none)

    // Expectations
    for (uint i = 0; i < MATRIX_COLS; i++) {
        mock().expectOneCall("gpio_init").withParameter("gpio", i+1);

        mock().expectOneCall("gpio_set_dir")
        .withParameter("gpio", i+1)
        .withParameter("out", true);

        mock().expectOneCall("gpio_put")
        .withParameter("gpio", i+1)
        .withParameter("value", false);
    }

    for (uint i = 0; i < MATRIX_ROWS; i++) {
        mock().expectOneCall("gpio_init").withParameter("gpio", MATRIX_COLS+i+1);

        mock().expectOneCall("gpio_set_dir")
        .withParameter("gpio", MATRIX_COLS+i+1)
        .withParameter("out", false);

        mock().expectOneCall("gpio_pull_down").withParameter("gpio", MATRIX_COLS+i+1);
    }

    // Production call
    matrix_init();

    // Checks
    // (none)
}

TEST(matrix, matrix_reset_clears_internal_state)
{
    // Setup
    // Set the internal state to anything but zeros
    PlatformSpecificMemset(*internals->handled_bitmap, 0xaa, sizeof(*internals->handled_bitmap));
    PlatformSpecificMemset(*internals->pressed_bitmap, 0xbb, sizeof(*internals->pressed_bitmap));
    PlatformSpecificMemset(*internals->pressed_this_scan_bitmap, 0xcc, sizeof(*internals->pressed_this_scan_bitmap));
    PlatformSpecificMemset(*internals->prev_pressed_bitmap, 0xdd, sizeof(*internals->prev_pressed_bitmap));
    PlatformSpecificMemset(*internals->released_this_scan_bitmap, 0xee, sizeof(*internals->released_this_scan_bitmap));
    PlatformSpecificMemset(*internals->suppressed_until_release, 0xff, sizeof(*internals->suppressed_until_release));

    uint32_t expected_bitmap[MATRIX_ROWS] = {0}; // Everything should be all zeros afterwards

    // Expectations
    // (none, just calls to memset)

    // Production call
    matrix_reset();

    // Checks
    MEMCMP_EQUAL(expected_bitmap, *internals->handled_bitmap, sizeof(*internals->handled_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->pressed_bitmap, sizeof(*internals->pressed_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->pressed_this_scan_bitmap, sizeof(*internals->pressed_this_scan_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->prev_pressed_bitmap, sizeof(*internals->prev_pressed_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->released_this_scan_bitmap, sizeof(*internals->released_this_scan_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->suppressed_until_release, sizeof(*internals->suppressed_until_release));
}

TEST(matrix, matrix_scan_cold_no_keys_pressed)
{
    // Setup
    uint32_t expected_bitmap[MATRIX_ROWS] = {0}; // Everything should be all zeros afterwards

    // Expectations
    for (uint i = 0; i < MATRIX_COLS; i++) {
        mock().expectOneCall("gpio_put").withParameter("gpio", i+1).withParameter("value", true);

        // matrix_settle_delay()
        {
            mock().expectNCalls(MATRIX_SETTLE_ITERATIONS, "asm: nop\n");
        }

        for (uint j = 0; j < MATRIX_ROWS; j++) {
            mock().expectOneCall("gpio_get").withParameter("gpio", MATRIX_COLS+j+1).andReturnValue(false);
        }

        mock().expectOneCall("gpio_put").withParameter("gpio", i+1).withParameter("value", false);

        // matrix_settle_delay()
        {
            mock().expectNCalls(MATRIX_SETTLE_ITERATIONS, "asm: nop\n");
        }
    }

    mock().expectOneCall("keyboard_post_scan");

    // Production call
    matrix_scan();

    // Checks
    MEMCMP_EQUAL(expected_bitmap, *internals->handled_bitmap, sizeof(*internals->handled_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->pressed_bitmap, sizeof(*internals->pressed_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->pressed_this_scan_bitmap, sizeof(*internals->pressed_this_scan_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->prev_pressed_bitmap, sizeof(*internals->prev_pressed_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->released_this_scan_bitmap, sizeof(*internals->released_this_scan_bitmap));
    MEMCMP_EQUAL(expected_bitmap, *internals->suppressed_until_release, sizeof(*internals->suppressed_until_release));
}

TEST(matrix, matrix_scan_keys_pressed)
{
    // Setup
    std::vector<uint32_t> keys_pressed[MATRIX_ROWS] = {
        {2, 3},
        {0},
        {4, 5, 7},
        {9}
    };
    uint32_t expected_pressed[MATRIX_ROWS] = {
        (1 << 2) | (1 << 3),
        (1 << 0),
        (1 << 4) | (1 << 5) | (1 << 7),
        (1 << 9)
    };
    uint32_t expected_zeros[MATRIX_ROWS] = {0};

    // Expectations
    for (uint i = 0; i < MATRIX_COLS; i++) {
        mock().expectOneCall("gpio_put").withParameter("gpio", i+1).withParameter("value", true);

        // matrix_settle_delay()
        {
            mock().expectNCalls(MATRIX_SETTLE_ITERATIONS, "asm: nop\n");
        }

        for (uint j = 0; j < MATRIX_ROWS; j++) {
            bool pin_state = std::find(keys_pressed[j].begin(), keys_pressed[j].end(), i) != keys_pressed[j].end();
            mock().expectOneCall("gpio_get").withParameter("gpio", MATRIX_COLS+j+1).andReturnValue(pin_state);
        }

        mock().expectOneCall("gpio_put").withParameter("gpio", i+1).withParameter("value", false);

        // matrix_settle_delay()
        {
            mock().expectNCalls(MATRIX_SETTLE_ITERATIONS, "asm: nop\n");
        }
    }

    mock().expectOneCall("keyboard_post_scan");

    // Production call
    matrix_scan();

    // Checks
    MEMCMP_EQUAL(expected_zeros, *internals->handled_bitmap, sizeof(*internals->handled_bitmap));
    MEMCMP_EQUAL(expected_pressed, *internals->pressed_bitmap, sizeof(*internals->pressed_bitmap));
    MEMCMP_EQUAL(expected_pressed, *internals->pressed_this_scan_bitmap, sizeof(*internals->pressed_this_scan_bitmap));
    MEMCMP_EQUAL(expected_zeros, *internals->prev_pressed_bitmap, sizeof(*internals->prev_pressed_bitmap));
    MEMCMP_EQUAL(expected_zeros, *internals->released_this_scan_bitmap, sizeof(*internals->released_this_scan_bitmap));
    MEMCMP_EQUAL(expected_zeros, *internals->suppressed_until_release, sizeof(*internals->suppressed_until_release));
}

TEST(matrix, matrix_scan_delta)
{
    // Setup
    (*internals->pressed_bitmap)[0] = (1 << 2) | (1 << 3);
    (*internals->pressed_bitmap)[1] = (1 << 0);
    (*internals->pressed_bitmap)[2] = (1 << 4) | (1 << 5) | (1 << 7);
    (*internals->pressed_bitmap)[3] = (1 << 9);

    std::vector<uint32_t> keys_pressed[MATRIX_ROWS] = {
        {3},
        {0},
        {7},
        {9, 10}
    };
    uint32_t expected_prev_pressed[MATRIX_ROWS] = {
        (1 << 2) | (1 << 3),
        (1 << 0),
        (1 << 4) | (1 << 5) | (1 << 7),
        (1 << 9)
    };
    uint32_t expected_pressed[MATRIX_ROWS] = {
        (1 << 3),
        (1 << 0),
        (1 << 7),
        (1 << 9) | (1 << 10)
    };
    uint32_t expected_pressed_this_scan[MATRIX_ROWS] = {
        0,
        0,
        0,
        (1 << 10)
    };
    uint32_t expected_released[MATRIX_ROWS] = {
        (1 << 2),
        0,
        (1 << 4) | (1 << 5),
        0
    };
    uint32_t expected_zeros[MATRIX_ROWS] = {0};

    // Expectations
    for (uint i = 0; i < MATRIX_COLS; i++) {
        mock().expectOneCall("gpio_put").withParameter("gpio", i+1).withParameter("value", true);

        // matrix_settle_delay()
        {
            mock().expectNCalls(MATRIX_SETTLE_ITERATIONS, "asm: nop\n");
        }

        for (uint j = 0; j < MATRIX_ROWS; j++) {
            bool pin_state = std::find(keys_pressed[j].begin(), keys_pressed[j].end(), i) != keys_pressed[j].end();
            mock().expectOneCall("gpio_get").withParameter("gpio", MATRIX_COLS+j+1).andReturnValue(pin_state);
        }

        mock().expectOneCall("gpio_put").withParameter("gpio", i+1).withParameter("value", false);

        // matrix_settle_delay()
        {
            mock().expectNCalls(MATRIX_SETTLE_ITERATIONS, "asm: nop\n");
        }
    }

    mock().expectOneCall("keyboard_post_scan");

    // Production call
    matrix_scan();

    // Checks
    MEMCMP_EQUAL(expected_zeros, *internals->handled_bitmap, sizeof(*internals->handled_bitmap));
    MEMCMP_EQUAL(expected_pressed, *internals->pressed_bitmap, sizeof(*internals->pressed_bitmap));
    MEMCMP_EQUAL(expected_pressed_this_scan, *internals->pressed_this_scan_bitmap, sizeof(*internals->pressed_this_scan_bitmap));
    MEMCMP_EQUAL(expected_prev_pressed, *internals->prev_pressed_bitmap, sizeof(*internals->prev_pressed_bitmap));
    MEMCMP_EQUAL(expected_released, *internals->released_this_scan_bitmap, sizeof(*internals->released_this_scan_bitmap));
    MEMCMP_EQUAL(expected_zeros, *internals->suppressed_until_release, sizeof(*internals->suppressed_until_release));
}

TEST(matrix, matrix_key_pressed_nothing_pressed_keep_handled)
{
    // Setup
    // (none)

    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Expectations
            // (none)

            // Production call
            bool result = matrix_key_pressed(row, col, true);

            // Checks
            CHECK_EQUAL(false, result);
        }
    }
}

TEST(matrix, matrix_key_pressed_everything_pressed_everything_handled_keep_handled)
{
    // Setup
    (*internals->pressed_bitmap)[0] = ((1 << MATRIX_COLS) - 1);
    (*internals->pressed_bitmap)[1] = ((1 << MATRIX_COLS) - 1);
    (*internals->pressed_bitmap)[2] = ((1 << MATRIX_COLS) - 1);
    (*internals->pressed_bitmap)[3] = ((1 << MATRIX_COLS) - 1);

    (*internals->handled_bitmap)[0] = ((1 << MATRIX_COLS) - 1);
    (*internals->handled_bitmap)[1] = ((1 << MATRIX_COLS) - 1);
    (*internals->handled_bitmap)[2] = ((1 << MATRIX_COLS) - 1);
    (*internals->handled_bitmap)[3] = ((1 << MATRIX_COLS) - 1);

    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Expectations
            // (none)

            // Production call
            bool result = matrix_key_pressed(row, col, true);

            // Checks
            CHECK_EQUAL(true, result);
        }
    }
}

TEST(matrix, matrix_key_pressed_some_pressed_some_handled_keep_handled)
{
    // Setup
    (*internals->pressed_bitmap)[0] = (1 << 0) | (1 << 1);
    (*internals->pressed_bitmap)[1] = (1 << 2) | (1 << 3);
    (*internals->pressed_bitmap)[2] = (1 << 4) | (1 << 5);
    (*internals->pressed_bitmap)[3] = (1 << 6) | (1 << 7);

    (*internals->handled_bitmap)[0] = (1 << 0);
    (*internals->handled_bitmap)[1] = (1 << 3);
    (*internals->handled_bitmap)[2] = (1 << 4);
    (*internals->handled_bitmap)[3] = (1 << 7);

    #define RC(row, col) ((uint64_t)row << 32ull) | (uint64_t)col

    std::unordered_map<uint64_t, bool> expect_press{
        {RC(0, 0), true}, {RC(0, 1), true},
        {RC(1, 2), true}, {RC(1, 3), true},
        {RC(2, 4), true}, {RC(2, 5), true},
        {RC(3, 6), true}, {RC(3, 7), true},
    };

    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Expectations
            // (none)

            // Production call
            bool result = matrix_key_pressed(row, col, true);

            // Checks
            CHECK_EQUAL(expect_press.contains(RC(row, col)), result);
        }
    }
}

TEST(matrix, matrix_key_pressed_some_pressed_some_handled_ignore_handled)
{
    // Setup
    (*internals->pressed_bitmap)[0] = (1 << 0) | (1 << 1);
    (*internals->pressed_bitmap)[1] = (1 << 2) | (1 << 3);
    (*internals->pressed_bitmap)[2] = (1 << 4) | (1 << 5);
    (*internals->pressed_bitmap)[3] = (1 << 6) | (1 << 7);

    (*internals->handled_bitmap)[0] = (1 << 0);
    (*internals->handled_bitmap)[1] = (1 << 3);
    (*internals->handled_bitmap)[2] = (1 << 4);
    (*internals->handled_bitmap)[3] = (1 << 7);

    #define RC(row, col) ((uint64_t)row << 32ull) | (uint64_t)col

    std::unordered_map<uint64_t, bool> expect_press{
        {RC(0, 1), true},
        {RC(1, 2), true},
        {RC(2, 5), true},
        {RC(3, 6), true},
    };

    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Expectations
            // (none)

            // Production call
            bool result = matrix_key_pressed(row, col, false);

            // Checks
            CHECK_EQUAL(expect_press.contains(RC(row, col)), result);
        }
    }
}

TEST(matrix, matrix_key_pressed_this_scan_reflects_pressed_this_scan)
{
    // Setup
    const uint32_t valid_keys = ((1 << MATRIX_COLS) - 1);
    (*internals->pressed_this_scan_bitmap)[0] = 0xAAAAAAAA & valid_keys;
    (*internals->pressed_this_scan_bitmap)[1] = 0xBBBBBBBB & valid_keys;
    (*internals->pressed_this_scan_bitmap)[2] = 0xCCCCCCCC & valid_keys;
    (*internals->pressed_this_scan_bitmap)[3] = 0xDDDDDDDD & valid_keys;

    uint32_t query_results[MATRIX_ROWS] = {0};

    // Expectations
    // (none)

    // Production calls
    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            bool result = matrix_key_pressed_this_scan(row, col);
            query_results[row] |= (result << col);
        }
    }

    // Checks
    MEMCMP_EQUAL(*internals->pressed_this_scan_bitmap, query_results, sizeof(*internals->pressed_this_scan_bitmap));
}

TEST(matrix, matrix_key_released_this_scan_reflects_released_this_scan)
{
    // Setup
    const uint32_t valid_keys = ((1 << MATRIX_COLS) - 1);
    (*internals->released_this_scan_bitmap)[0] = 0xAAAAAAAA & valid_keys;
    (*internals->released_this_scan_bitmap)[1] = 0xBBBBBBBB & valid_keys;
    (*internals->released_this_scan_bitmap)[2] = 0xCCCCCCCC & valid_keys;
    (*internals->released_this_scan_bitmap)[3] = 0xDDDDDDDD & valid_keys;

    uint32_t query_results[MATRIX_ROWS] = {0};

    // Expectations
    // (none)

    // Production calls
    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            bool result = matrix_key_released_this_scan(row, col);
            query_results[row] |= (result << col);
        }
    }

    // Checks
    MEMCMP_EQUAL(*internals->released_this_scan_bitmap, query_results, sizeof(*internals->released_this_scan_bitmap));
}

TEST(matrix, matrix_suppress_held_until_release_adds_pressed_keys_to_the_suppressed_list)
{
    // Setup
    (*internals->suppressed_until_release)[0] = (1 << 0);
    (*internals->suppressed_until_release)[1] = (1 << 1);
    (*internals->suppressed_until_release)[2] = (1 << 2);
    (*internals->suppressed_until_release)[3] = (1 << 3);

    (*internals->pressed_bitmap)[0] = (1 << 5);
    (*internals->pressed_bitmap)[1] = (1 << 6);
    (*internals->pressed_bitmap)[2] = (1 << 7);
    (*internals->pressed_bitmap)[3] = (1 << 8);

    uint32_t expected_new_suppressed[MATRIX_ROWS] = {
        (1 << 0) | (1 << 5),
        (1 << 1) | (1 << 6),
        (1 << 2) | (1 << 7),
        (1 << 3) | (1 << 8),
    };

    // Expectations
    // (none)

    // Production calls
    matrix_suppress_held_until_release();

    // Checks
    MEMCMP_EQUAL(*internals->suppressed_until_release, expected_new_suppressed, sizeof(*internals->suppressed_until_release));
}

TEST(matrix, matrix_suppress_key_until_release_adds_key_to_suppressed_bitmap)
{
    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Setup
            (*internals->suppressed_until_release)[0] = (1 << 0);
            (*internals->suppressed_until_release)[1] = (1 << 1);
            (*internals->suppressed_until_release)[2] = (1 << 2);
            (*internals->suppressed_until_release)[3] = (1 << 3);

            // (make sure the key is actually marked as pressed)
            (*internals->pressed_bitmap)[row] = (1 << col);

            // Expectations
            // (none)

            // Production calls
            matrix_suppress_key_until_release(row, col);

            // Checks
            if (row == 0) {
                CHECK_EQUAL((1u << 0u) | (1u << col), (*internals->suppressed_until_release)[0]);
            } else {
                CHECK_EQUAL((1u << 0u), (*internals->suppressed_until_release)[0]);
            }

            if (row == 1) {
                CHECK_EQUAL((1u << 1u) | (1u << col), (*internals->suppressed_until_release)[1]);
            } else {
                CHECK_EQUAL((1u << 1u), (*internals->suppressed_until_release)[1]);
            }

            if (row == 2) {
                CHECK_EQUAL((1u << 2u) | (1u << col), (*internals->suppressed_until_release)[2]);
            } else {
                CHECK_EQUAL((1u << 2u), (*internals->suppressed_until_release)[2]);
            }

            if (row == 3) {
                CHECK_EQUAL((1u << 3u) | (1u << col), (*internals->suppressed_until_release)[3]);
            } else {
                CHECK_EQUAL((1u << 3u), (*internals->suppressed_until_release)[3]);
            }
        }
    }
}

TEST(matrix, matrix_mark_key_as_handled_adds_key_to_handled_bitmap)
{
    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Setup
            (*internals->handled_bitmap)[0] = (1 << 0);
            (*internals->handled_bitmap)[1] = (1 << 1);
            (*internals->handled_bitmap)[2] = (1 << 2);
            (*internals->handled_bitmap)[3] = (1 << 3);

            // Expectations
            // (none)

            // Production calls
            matrix_mark_key_as_handled(row, col);

            // Checks
            if (row == 0) {
                CHECK_EQUAL((1u << 0u) | (1u << col), (*internals->handled_bitmap)[0]);
            } else {
                CHECK_EQUAL((1u << 0u), (*internals->handled_bitmap)[0]);
            }

            if (row == 1) {
                CHECK_EQUAL((1u << 1u) | (1u << col), (*internals->handled_bitmap)[1]);
            } else {
                CHECK_EQUAL((1u << 1u), (*internals->handled_bitmap)[1]);
            }

            if (row == 2) {
                CHECK_EQUAL((1u << 2u) | (1u << col), (*internals->handled_bitmap)[2]);
            } else {
                CHECK_EQUAL((1u << 2u), (*internals->handled_bitmap)[2]);
            }

            if (row == 3) {
                CHECK_EQUAL((1u << 3u) | (1u << col), (*internals->handled_bitmap)[3]);
            } else {
                CHECK_EQUAL((1u << 3u), (*internals->handled_bitmap)[3]);
            }
        }
    }
}

TEST(matrix, matrix_mark_key_as_unhandled_clears_key_in_handled_bitmap)
{
    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint32_t col = 0; col < MATRIX_COLS; col++) {
            // Setup
            (*internals->handled_bitmap)[0] = 0xffffffff;
            (*internals->handled_bitmap)[1] = 0xffffffff;
            (*internals->handled_bitmap)[2] = 0xffffffff;
            (*internals->handled_bitmap)[3] = 0xffffffff;

            // Expectations
            // (none)

            // Production calls
            matrix_mark_key_as_unhandled(row, col);

            // Checks
            if (row == 0) {
                CHECK_EQUAL((0xffffffffu & ~(1u << col)), (*internals->handled_bitmap)[0]);
            } else {
                CHECK_EQUAL(0xffffffffu, (*internals->handled_bitmap)[0]);
            }

            if (row == 1) {
                CHECK_EQUAL((0xffffffffu & ~(1u << col)), (*internals->handled_bitmap)[1]);
            } else {
                CHECK_EQUAL(0xffffffffu, (*internals->handled_bitmap)[1]);
            }

            if (row == 2) {
                CHECK_EQUAL((0xffffffffu & ~(1u << col)), (*internals->handled_bitmap)[2]);
            } else {
                CHECK_EQUAL(0xffffffffu, (*internals->handled_bitmap)[2]);
            }

            if (row == 3) {
                CHECK_EQUAL((0xffffffffu & ~(1u << col)), (*internals->handled_bitmap)[3]);
            } else {
                CHECK_EQUAL(0xffffffffu, (*internals->handled_bitmap)[3]);
            }
        }
    }
}

TEST(matrix, matrix_get_pressed_bitmap_returns_ptr)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production calls
    const uint32_t* ptr = matrix_get_pressed_bitmap();

    // Checks
    POINTERS_EQUAL(*internals->pressed_bitmap, ptr);
}

TEST(matrix, matrix_get_handled_bitmap_returns_ptr)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production calls
    const uint32_t* ptr = matrix_get_handled_bitmap();

    // Checks
    POINTERS_EQUAL(*internals->handled_bitmap, ptr);
}

TEST(matrix, matrix_get_released_this_scan_bitmap_returns_ptr)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production calls
    const uint32_t* ptr = matrix_get_released_this_scan_bitmap();

    // Checks
    POINTERS_EQUAL(*internals->released_this_scan_bitmap, ptr);
}

TEST(matrix, matrix_get_pressed_this_scan_bitmap_returns_ptr)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production calls
    const uint32_t* ptr = matrix_get_pressed_this_scan_bitmap();

    // Checks
    POINTERS_EQUAL(*internals->pressed_this_scan_bitmap, ptr);
}

TEST(matrix, matrix_get_col_gpio_translates_column_index_to_pin)
{
    // Setup
    for (uint32_t col = 0; col < MATRIX_COLS; col++) {

        // Expectations
        // (none)

        // Production calls
        uint pin = matrix_get_col_gpio(col);

        // Checks
        CHECK_EQUAL(col+1, pin);
    }
}

TEST(matrix, matrix_get_row_gpio_translates_row_index_to_pin)
{
    // Setup
    for (uint32_t row = 0; row < MATRIX_ROWS; row++) {

        // Expectations
        // (none)

        // Production calls
        uint pin = matrix_get_row_gpio(row);

        // Checks
        CHECK_EQUAL(row+1+MATRIX_COLS, pin);
    }
}
