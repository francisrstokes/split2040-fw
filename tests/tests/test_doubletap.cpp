#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTest/PlatformSpecificFunctions.h"

#include "mock_doubletap.h"
#include "mock_keyboard.h"
#include "mock_matrix.h"
#include "mock_ll_alloc.h"

#include <vector>

TEST_GROUP(double_tap) {

    DoubleTapInternals_t* internals = mock_double_tap_get_internals();

    void setup() {
        mock_lla_use_mocks(false);
        mock_double_tap_use_mocks(false);

        reset_environment();

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_double_tap_use_mocks(true);
        mock_lla_use_mocks(true);

        reset_environment();
    }

    void reset_environment(void) {
        PlatformSpecificMemset(internals->double_taps, 0, sizeof(*internals->double_taps));

        mock_lla_get_prod_fn_ptr_struct()->lla_init(
            &internals->double_taps->allocator,
            internals->double_taps->data_array,
            internals->double_taps->node_array,
            DOUBLE_TAP_MAX,
            sizeof(double_tap_data_t)
        );
    }

    uint32_t active_dts_to_vector(std::vector<double_tap_data_t*>& vec) {
        uint32_t items_written = 0;
        ll_node_t* node = internals->double_taps->allocator.active_head;

        while (node != NULL) {
            ++items_written;
            vec.push_back((double_tap_data_t*)node->data);
            node = node->next;
        }

        return items_written;
    }

    void insert_test_dt(double_tap_data_t& dt) {
        if (internals->double_taps->allocator.free_head == NULL) {
            FAIL("insert_test_dt: No available nodes");
        }
        ll_node_t* dt_node = lla_alloc_tail(&internals->double_taps->allocator);
        double_tap_data_t* dt_data = (double_tap_data_t*)dt_node->data;
        *dt_data = dt;
    }
};

TEST(double_tap, double_tap_init_initialises_allocator)
{
    // Setup
    mock_lla_use_mocks(true);

    // Expectations
    mock().expectOneCall("lla_init")
    .withParameter("alloc", &internals->double_taps->allocator)
    .withParameter("data_block", internals->double_taps->data_array)
    .withParameter("node_block", internals->double_taps->node_array)
    .withParameter("capacity", DOUBLE_TAP_MAX)
    .withParameter("elem_size", sizeof(double_tap_data_t));

    // Production call
    double_tap_init();

    // Checks
    // (none)
}

TEST(double_tap, double_tap_reset_clears_all_ongoing_dts)
{
    // Setup
    mock_lla_use_mocks(true);

    // Expectations
    mock().expectOneCall("lla_free_all").withParameter("alloc", &internals->double_taps->allocator);

    // Production call
    double_tap_reset();

    // Checks
    // (none)
}

TEST(double_tap, double_tap_on_key_press_ignores_non_dt_keys)
{
    // Setup
    keymap_entry_t key = KC_A;

    // Expectations
    // (none)

    // Production call
    bool result = double_tap_on_key_press(4, 2, key);

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(double_tap, double_tap_on_key_press_first_press)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active()
    {
        // none, no dts active
    }

    mock().expectOneCall("keyboard_get_current_layer").andReturnValue(0);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 4).withParameter("col", 2);

    // Production call
    bool result = double_tap_on_key_press(4, 2, key);

    // Checks
    CHECK_EQUAL(true, result);

    CHECK_EQUAL(1, active_dts_to_vector(active_dts));
    CHECK_EQUAL(0, active_dts[0]->time_since_first_tap);
    CHECK_EQUAL(4, active_dts[0]->row);
    CHECK_EQUAL(2, active_dts[0]->col);
    CHECK_EQUAL(0, active_dts[0]->layer);
    CHECK_EQUAL(dt_state_wait_first_release, active_dts[0]->state);
}

TEST(double_tap, double_tap_on_key_press_second_press)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = MATRIX_SCAN_INTERVAL_MS * 5,
        .state = dt_state_wait_second_press,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    // Production call
    bool result = double_tap_on_key_press(4, 2, key);

    // Checks
    CHECK_EQUAL(true, result);

    CHECK_EQUAL(1, active_dts_to_vector(active_dts));
    CHECK_EQUAL(MATRIX_SCAN_INTERVAL_MS*5, active_dts[0]->time_since_first_tap);
    CHECK_EQUAL(4, active_dts[0]->row);
    CHECK_EQUAL(2, active_dts[0]->col);
    CHECK_EQUAL(0, active_dts[0]->layer);
    CHECK_EQUAL(dt_state_double_tap, active_dts[0]->state);
}

TEST(double_tap, double_tap_on_key_press_different_key)
{
    // Setup
    keymap_entry_t key1 = DT(KC_A, KC_B, 0);
    keymap_entry_t key2 = DT(KC_C, KC_D, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = MATRIX_SCAN_INTERVAL_MS * 5,
        .state = dt_state_wait_first_release,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key1);
    }

    mock().expectOneCall("keyboard_get_current_layer").andReturnValue(0);
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 5).withParameter("col", 3);


    // Production call
    bool result = double_tap_on_key_press(5, 3, key2);

    // Checks
    CHECK_EQUAL(true, result);

    CHECK_EQUAL(2, active_dts_to_vector(active_dts));
    CHECK_EQUAL(0, active_dts[1]->time_since_first_tap);
    CHECK_EQUAL(5, active_dts[1]->row);
    CHECK_EQUAL(3, active_dts[1]->col);
    CHECK_EQUAL(0, active_dts[1]->layer);
    CHECK_EQUAL(dt_state_wait_first_release, active_dts[1]->state);
}

TEST(double_tap, double_tap_on_key_release_ignores_non_dt_keys)
{
    // Setup
    keymap_entry_t key = KC_A;

    // Expectations
    // (none)

    // Production call
    bool result = double_tap_on_key_release(4, 2, key);

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(double_tap, double_tap_on_key_release_was_waiting)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = MATRIX_SCAN_INTERVAL_MS * 5,
        .state = dt_state_wait_first_release,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    // Production call
    bool result = double_tap_on_key_release(4, 2, key);

    // Checks
    CHECK_EQUAL(true, result);

    CHECK_EQUAL(1, active_dts_to_vector(active_dts));
    CHECK_EQUAL(MATRIX_SCAN_INTERVAL_MS*5, active_dts[0]->time_since_first_tap);
    CHECK_EQUAL(4, active_dts[0]->row);
    CHECK_EQUAL(2, active_dts[0]->col);
    CHECK_EQUAL(0, active_dts[0]->layer);
    CHECK_EQUAL(dt_state_wait_second_press, active_dts[0]->state);
}

TEST(double_tap, double_tap_on_key_release_was_in_a_resolved_state)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = MATRIX_SCAN_INTERVAL_MS * 5,
        .state = dt_state_double_tap,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    // Production call
    bool result = double_tap_on_key_release(4, 2, key);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(0, active_dts_to_vector(active_dts));
}

TEST(double_tap, double_tap_update_no_ongoing_double_taps)
{
    // Setup
    // (none)

    // Expectations

    // Production call
    bool result = double_tap_update();

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(double_tap, double_tap_update_timer_not_yet_expired)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = 0,
        .state = dt_state_wait_first_release,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    // Production call
    bool result = double_tap_update();

    // Checks
    CHECK_EQUAL(true, result);

    CHECK_EQUAL(1, active_dts_to_vector(active_dts));
    CHECK_EQUAL(MATRIX_SCAN_INTERVAL_MS, active_dts[0]->time_since_first_tap);
    CHECK_EQUAL(4, active_dts[0]->row);
    CHECK_EQUAL(2, active_dts[0]->col);
    CHECK_EQUAL(0, active_dts[0]->layer);
    CHECK_EQUAL(dt_state_wait_first_release, active_dts[0]->state);
}

TEST(double_tap, double_tap_update_held_press_resolves_to_single_tap)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = (DOUBLE_TAP_DELAY_MS - MATRIX_SCAN_INTERVAL_MS),
        .state = dt_state_wait_first_release,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_A);

    // Production call
    bool result = double_tap_update();

    // Checks
    CHECK_EQUAL(false, result);

    CHECK_EQUAL(1, active_dts_to_vector(active_dts));
    CHECK_EQUAL(DOUBLE_TAP_DELAY_MS, active_dts[0]->time_since_first_tap);
    CHECK_EQUAL(4, active_dts[0]->row);
    CHECK_EQUAL(2, active_dts[0]->col);
    CHECK_EQUAL(0, active_dts[0]->layer);
    CHECK_EQUAL(dt_state_single_tap, active_dts[0]->state);
}

TEST(double_tap, double_tap_update_press_and_released_key_sends_key_and_clears_dt)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = (DOUBLE_TAP_DELAY_MS - MATRIX_SCAN_INTERVAL_MS),
        .state = dt_state_wait_second_press,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_A);

    // Production call
    bool result = double_tap_update();

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(0, active_dts_to_vector(active_dts));
}

TEST(double_tap, double_tap_update_double_tapped_key)
{
    // Setup
    keymap_entry_t key = DT(KC_A, KC_B, 0);

    double_tap_data_t dt = {
        .row = 4,
        .col = 2,
        .layer = 0,
        .time_since_first_tap = (DOUBLE_TAP_DELAY_MS - MATRIX_SCAN_INTERVAL_MS),
        .state = dt_state_double_tap,
    };
    insert_test_dt(dt);
    std::vector<double_tap_data_t*> active_dts{};

    // Expectations
    // double_tap_find_active() -> double_tap_is_matching_key()
    {
        mock().expectOneCall("keyboard_resolve_key_on_layer")
        .withParameter("row", 4)
        .withParameter("col", 2)
        .withParameter("layer", 0)
        .andReturnValue(key);
    }

    mock().expectOneCall("keyboard_send_key").withParameter("key", KC_B);

    // Production call
    bool result = double_tap_update();

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(1, active_dts_to_vector(active_dts));
    CHECK_EQUAL(DOUBLE_TAP_DELAY_MS, active_dts[0]->time_since_first_tap);
    CHECK_EQUAL(4, active_dts[0]->row);
    CHECK_EQUAL(2, active_dts[0]->col);
    CHECK_EQUAL(0, active_dts[0]->layer);
    CHECK_EQUAL(dt_state_double_tap, active_dts[0]->state);
}
