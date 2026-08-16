#include "CppUTest/TestHarness.h"
#include "CppUTest/PlatformSpecificFunctions.h"
#include "CppUTestExt/MockSupport.h"

#include "mock_combo.h"
#include "mock_keyboard.h"
#include "mock_matrix.h"
#include "mock_log.h"

TEST_GROUP(combo) {

    ComboInternals_t* internals = mock_combo_get_internals();

    void setup() {
        mock_combo_use_mocks(false);
        mock_log_expect_calls(false);

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_combo_use_mocks(true);
        mock_log_expect_calls(true);

        reset_environment();
    }

    void reset_environment(void) {
        *internals->combos = NULL;
    }
};

TEST(combo, combo_init_sets_combo_table_pointer)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };

    // Expectations
    // (none)

    // Production call
    combo_system()->init(combo_table);

    // Checks
    POINTERS_EQUAL(combo_table, *internals->combos);
}

TEST(combo, combo_reset_restores_valid_combos_to_inactive)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
        COMBO3(KC_C, KC_D, KC_ENTER, KC_ENTER),
    };
    *internals->combos = combo_table;
    uint8_t expected_positions[COMBO_KEYS_MAX] = {0};

    for (uint i = 0; i < 2; i++) {
        combo_table[i].state = combo_state_active;
        combo_table[i].time_since_first_press = 42;
        combo_table[i].keys_pressed_bitmask = (1 << 0) | (1 << 1);
        PlatformSpecificMemset(combo_table[i].key_positions, 0, sizeof(combo_table[i].key_positions));
    }

    // Expectations
    // (none)

    // Production call
    combo_system()->reset();

    // Checks
    PlatformSpecificMemset(expected_positions, 0xff, sizeof(expected_positions));

    for (uint i = 0; i < 2; i++) {
        CHECK_EQUAL(combo_state_inactive, combo_table[i].state);
        CHECK_EQUAL(0, combo_table[i].time_since_first_press);
        CHECK_EQUAL(0, combo_table[i].keys_pressed_bitmask);
        MEMCMP_EQUAL(expected_positions, combo_table[i].key_positions, sizeof(expected_positions));
    }
}

TEST(combo, combo_on_key_press_ignores_non_combo_keys)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_press(4, 2, KC_ENTER);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(combo_state_inactive, combo_table[0].state);
}

TEST(combo, combo_on_key_press_starts_combo_on_first_key_of_multiple)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Expectations
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 4).withParameter("col", 2);

    // Production call
    bool result = combo_system()->on_press(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(combo_state_active, combo_table[0].state);
    CHECK_EQUAL((1u << 0u), combo_table[0].keys_pressed_bitmask);
    CHECK_EQUAL(4, combo_table[0].key_positions[0].row);
    CHECK_EQUAL(2, combo_table[0].key_positions[0].col);
}

TEST(combo, combo_on_key_press_completes_combo_and_sends_output_key)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_active;
    combo_table[0].keys_pressed_bitmask = (1 << 0);
    combo_table[0].key_positions[0] = {4, 2};

    // Expectations
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 5).withParameter("col", 3);
    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_ENTER);

    // Production call
    bool result = combo_system()->on_press(5, 3, KC_B);

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(combo_state_wait_for_all_released, combo_table[0].state);
}

TEST(combo, combo_on_key_press_ignored_while_in_cooldown)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_cooldown;

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_press(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(combo_state_cooldown, combo_table[0].state);
    CHECK_EQUAL(0, combo_table[0].keys_pressed_bitmask);
}

TEST(combo, combo_on_key_press_remarks_key_while_waiting_for_all_released)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_wait_for_all_released;
    combo_table[0].keys_pressed_bitmask = 0;

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_press(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(combo_state_wait_for_all_released, combo_table[0].state);
    CHECK_EQUAL((1u << 0u), combo_table[0].keys_pressed_bitmask);
}

TEST(combo, combo_on_key_press_deactivates_other_combo_sharing_a_key_on_completion)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
        COMBO2(KC_A, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Expectations
    // Phase 1: Press the shared key and start both combos
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 0).withParameter("col", 0);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 0).withParameter("col", 0);

    // Phase 2: The key of combo[0] is pressed
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 1).withParameter("col", 1);
    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_ENTER);

    // Production calls
    combo_system()->on_press(0, 0, KC_A);
    bool result = combo_system()->on_press(1, 1, KC_B);

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(combo_state_wait_for_all_released, combo_table[0].state);
    CHECK_EQUAL(combo_state_cooldown, combo_table[1].state);
    CHECK_EQUAL(0, combo_table[1].time_since_first_press);
}

TEST(combo, combo_on_key_release_ignores_non_combo_keys)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_ENTER);

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(combo, combo_on_key_release_while_cooldown_clears_pressed_bit)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_cooldown;
    combo_table[0].keys_pressed_bitmask = (1 << 0);

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(combo_state_cooldown, combo_table[0].state);
    CHECK_EQUAL(0, combo_table[0].keys_pressed_bitmask);
}

TEST(combo, combo_on_key_release_while_wait_for_all_released_returns_to_inactive_once_all_released)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_wait_for_all_released;
    combo_table[0].keys_pressed_bitmask = (1 << 1);

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_B);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(0, combo_table[0].keys_pressed_bitmask);
    CHECK_EQUAL(combo_state_inactive, combo_table[0].state);
}

TEST(combo, combo_on_key_release_while_wait_for_all_released_stays_active_if_other_keys_held)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_wait_for_all_released;
    combo_table[0].keys_pressed_bitmask = (1 << 0) | (1 << 1);

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_B);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL((1u << 0u), combo_table[0].keys_pressed_bitmask);
    CHECK_EQUAL(combo_state_wait_for_all_released, combo_table[0].state);
}

TEST(combo, combo_on_key_release_while_single_held_returns_to_inactive)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_single_held;
    combo_table[0].held_index = 0;

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(combo_state_inactive, combo_table[0].state);
}

TEST(combo, combo_on_key_release_while_active_and_last_key_released_emits_single_key)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_active;
    combo_table[0].keys_pressed_bitmask = (1 << 0); // only A was ever pressed

    // Expectations
    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_A);

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(combo_state_inactive, combo_table[0].state);
}

TEST(combo, combo_on_key_release_while_active_with_other_keys_still_pressed_starts_cooldown)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO3(KC_A, KC_B, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_active;
    combo_table[0].keys_pressed_bitmask = (1 << 0) | (1 << 1); // A and B pressed, C not yet

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_B);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(combo_state_cooldown, combo_table[0].state);
    CHECK_EQUAL(0, combo_table[0].time_since_first_press);
    CHECK_EQUAL((1u << 0u), combo_table[0].keys_pressed_bitmask); // B's bit cleared, A's remains
}

TEST(combo, combo_on_key_release_while_inactive_does_nothing)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_inactive;

    // Expectations
    // (none)

    // Production call
    bool result = combo_system()->on_release(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(combo_state_inactive, combo_table[0].state);
}

TEST(combo, combo_update_skips_invalid_and_inactive_combos)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER), // left inactive
    };
    *internals->combos = combo_table;

    // Expectations
    // (none)

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
}

TEST(combo, combo_update_cooldown_marks_keys_as_handled_before_timeout)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_cooldown;
    combo_table[0].time_since_first_press = 0;
    combo_table[0].key_positions[0] = {4, 2};
    combo_table[0].key_positions[1] = {5, 3};

    // Expectations
    // (Assumes COMBO_CANCEL_SUPPRESS_MS is more than one MATRIX_SCAN_INTERVAL_MS)
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 4).withParameter("col", 2);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 5).withParameter("col", 3);

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_cooldown, combo_table[0].state);
    CHECK_EQUAL(MATRIX_SCAN_INTERVAL_MS, combo_table[0].time_since_first_press);
}

TEST(combo, combo_update_cooldown_becomes_inactive_after_timeout)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_cooldown;
    combo_table[0].time_since_first_press = (uint8_t)(COMBO_CANCEL_SUPPRESS_MS - 1);

    // Expectations
    // (none)

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_inactive, combo_table[0].state);
}

TEST(combo, combo_update_wait_for_all_released_marks_keys_as_handled)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_wait_for_all_released;
    combo_table[0].key_positions[0] = {4, 2};
    combo_table[0].key_positions[1] = {5, 3};

    // Expectations
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 4).withParameter("col", 2);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 5).withParameter("col", 3);

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_wait_for_all_released, combo_table[0].state);
}

TEST(combo, combo_update_active_before_delay_only_tracks_time)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_active;
    combo_table[0].time_since_first_press = 0;
    combo_table[0].keys_pressed_bitmask = (1 << 0);

    // Expectations
    // (none)

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_active, combo_table[0].state);
    CHECK_EQUAL(MATRIX_SCAN_INTERVAL_MS, combo_table[0].time_since_first_press);
}

TEST(combo, combo_update_active_after_delay_with_single_key_emits_and_becomes_single_held)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_active;
    combo_table[0].time_since_first_press = (uint8_t)(COMBO_DELAY_MS - 1);
    combo_table[0].keys_pressed_bitmask = (1 << 0); // only A held

    // Expectations
    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_A);

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_single_held, combo_table[0].state);
    CHECK_EQUAL(0, combo_table[0].held_index);
}

TEST(combo, combo_update_active_after_delay_with_multiple_keys_starts_cooldown)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO3(KC_A, KC_B, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_active;
    combo_table[0].time_since_first_press = (uint8_t)(COMBO_DELAY_MS - 1);
    combo_table[0].keys_pressed_bitmask = (1 << 0) | (1 << 1); // A and B held, C never pressed
    combo_table[0].key_positions[0] = {4, 2};
    combo_table[0].key_positions[1] = {5, 3};
    combo_table[0].key_positions[2] = {0xff, 0xff}; // Never pressed, so set to the initial value of (0xff, 0xff)

    // Expectations
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 4).withParameter("col", 2);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 5).withParameter("col", 3);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 0xff).withParameter("col", 0xff);

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_cooldown, combo_table[0].state);
    CHECK_EQUAL(0, combo_table[0].time_since_first_press);
}

TEST(combo, combo_update_single_held_repeats_output_key)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
    };
    *internals->combos = combo_table;

    combo_table[0].state = combo_state_single_held;
    combo_table[0].held_index = 1;

    // Expectations
    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_B);

    // Production call
    bool active = combo_system()->update();

    // Checks
    CHECK_EQUAL(false, active);
    CHECK_EQUAL(combo_state_single_held, combo_table[0].state);
}

TEST(combo, combo_get_key_index_finds_key_position)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO3(KC_A, KC_B, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Checks
    LONGS_EQUAL(0, internals->combo_get_key_index(0, KC_A));
    LONGS_EQUAL(1, internals->combo_get_key_index(0, KC_B));
    LONGS_EQUAL(2, internals->combo_get_key_index(0, KC_C));
}

TEST(combo, combo_get_key_index_returns_negative_one_for_unrelated_key)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO3(KC_A, KC_B, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Checks
    LONGS_EQUAL(-1, internals->combo_get_key_index(0, KC_ENTER));
}

TEST(combo, combo_is_complete_true_only_when_all_defined_keys_pressed)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO3(KC_A, KC_B, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Checks
    combo_table[0].keys_pressed_bitmask = (1 << 0) | (1 << 1);
    CHECK_FALSE(internals->combo_is_complete(0));

    combo_table[0].keys_pressed_bitmask = (1 << 0) | (1 << 1) | (1 << 2);
    CHECK(internals->combo_is_complete(0));
}

TEST(combo, combo_get_single_pressed_index_identifies_lone_key_or_returns_negative_one)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO3(KC_A, KC_B, KC_C, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Checks
    combo_table[0].keys_pressed_bitmask = 0;
    LONGS_EQUAL(-1, internals->combo_get_single_pressed_index(0));

    combo_table[0].keys_pressed_bitmask = (1 << 1);
    LONGS_EQUAL(1, internals->combo_get_single_pressed_index(0));

    combo_table[0].keys_pressed_bitmask = (1 << 0) | (1 << 2);
    LONGS_EQUAL(-1, internals->combo_get_single_pressed_index(0));
}

TEST(combo, combo_deactivate_unfinished_overlapping_combos_puts_all_combos_with_overlapping_keys_into_cooldown)
{
    // Setup
    combo_t combo_table[COMBO_MAX] = {
        COMBO2(KC_A, KC_B, KC_ENTER),
        COMBO2(KC_A, KC_C, KC_ENTER),
        COMBO2(KC_A, KC_D, KC_ENTER),
    };
    *internals->combos = combo_table;

    // Production call
    internals->combo_deactivate_unfinished_overlapping_combos(0);

    // Checks
    CHECK_EQUAL(combo_state_cooldown, combo_table[1].state);
    CHECK_EQUAL(combo_state_cooldown, combo_table[2].state);
}
