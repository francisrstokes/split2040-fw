#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "mock_layers.h"
#include "mock_keyboard.h"

TEST_GROUP(layers) {

    LayersInternals_t* internals = mock_layers_get_internals();

    void setup() {
        mock_layers_use_mocks(false);

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_layers_use_mocks(true);

        reset_environment();
    }

    void reset_environment(void) {
        *internals->layer_state = {0};
    }
};

TEST(layers, layers_on_key_press_ignores_non_layer_keys)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production call
    bool result = layers_on_key_press(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(0, internals->layer_state->current);
}

TEST(layers, layers_on_key_press_switches_layer)
{
    // Setup
    // (none)

    // Expectations
    mock().expectOneCall("matrix_mark_key_as_handled").withParameter("row", 4).withParameter("col", 2);

    // Production call
    bool result = layers_on_key_press(4, 2, MO(5));

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(5, internals->layer_state->current);
}

TEST(layers, layers_on_key_release_ignores_non_layer_keys)
{
    // Setup
    internals->layer_state->current = 42;

    // Expectations
    // (none)

    // Production call
    bool result = layers_on_key_release(4, 2, KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(42, internals->layer_state->current);
}

TEST(layers, layers_on_key_release_switches_layer_back_to_base)
{
    // Setup
    internals->layer_state->base = 2;
    internals->layer_state->current = 5;

    // Expectations
    mock().expectOneCall("matrix_suppress_held_until_release");

    // Production call
    bool result = layers_on_key_release(5, 2, MO(5));

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(2, internals->layer_state->current);
}

TEST(layers, layers_on_virtual_key_ignores_non_layer_keys)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production call
    bool result = layers_on_virtual_key(KC_A);

    // Checks
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(0, internals->layer_state->current);
}

TEST(layers, layers_on_virtual_key_changes_layer)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production call
    bool result = layers_on_virtual_key(MO(5));

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(5, internals->layer_state->current);
}

TEST(layers, layers_get_current_follows_state)
{
    // Setup
    internals->layer_state->current = 42;

    // Expectations
    // (none)

    // Production call
    uint8_t result = layers_get_current();

    // Checks
    CHECK_EQUAL(42, result);
}

TEST(layers, layers_get_base_follows_state)
{
    // Setup
    internals->layer_state->base = 42;

    // Expectations
    // (none)

    // Production call
    uint8_t result = layers_get_base();

    // Checks
    CHECK_EQUAL(42, result);
}

TEST(layers, layers_set_sets_layer_directly)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production call
    layers_set(42);

    // Checks
    CHECK_EQUAL(42, internals->layer_state->current);
}

TEST(layers, layers_reset_restores_base)
{
    // Setup
    internals->layer_state->base = 42;

    // Expectations
    // (none)

    // Production call
    layers_reset();

    // Checks
    CHECK_EQUAL(42, internals->layer_state->current);
}