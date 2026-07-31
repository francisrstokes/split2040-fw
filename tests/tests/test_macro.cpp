#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "mock_macro.h"
#include "mock_keyboard.h"

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))

TEST_GROUP(macro) {

    MacroInternals_t* internals = mock_macro_get_internals();

    void setup() {
        mock_macro_use_mocks(false);

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_macro_use_mocks(true);

        reset_environment();
    }

    void reset_environment(void) {
        *internals->any_macro_active = false;
        *internals->macros = NULL;
    }
};

TEST(macro, macro_init_sets_macro_table_pointer)
{
    // Setup
    macro_t macro_table[] = {
        SEND_STRING("hello world", 12),
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
    };

    // Expectations
    // (none)

    // Production call
    macro_init(macro_table);

    // Checks
    POINTERS_EQUAL(macro_table, *internals->macros);
}

TEST(macro, macro_on_key_press_macro_starts)
{
    // Setup
    macro_t macro_table[] = {
        SEND_STRING("first", 6),
        SEND_STRING("second", 7),
        SEND_STRING("third", 6),
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
    };
    *internals->macros = macro_table;

    // Expectations
    // (none)

    // Production call
    bool started = macro_on_key_press(0, 0, MACRO(2));

    // Checks
    CHECK(started);
    CHECK(*internals->any_macro_active);
    CHECK_FALSE(macro_table[0].active);
    CHECK_FALSE(macro_table[1].active);
    CHECK(macro_table[2].active);
    LONGS_EQUAL(0, macro_table[2].send_string.index);
}

TEST(macro, macro_on_key_press_non_macro_key)
{
    // Setup
    macro_t macro_table[] = {
        SEND_STRING("hello world", 12),
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
    };
    *internals->macros = macro_table;

    // Expectations
    // (none)

    // Production call
    bool started = macro_on_key_press(0, 0, KC_ENTER);

    // Checks
    CHECK_FALSE(started);
    CHECK_FALSE(*internals->any_macro_active);
    CHECK_FALSE(macro_table[0].active);
}

TEST(macro, macro_update_sends_macro_keys)
{
    // Setup
    macro_t macro_table[] = {
        SEND_STRING("first", 6),
        SEND_STRING("second", 7),
        SEND_STRING("Hello World123#$%", 18),
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
        MACRO_UNUSED,
    };
    *internals->macros = macro_table;
    macro_table[2].active = true;

    const uint32_t NO_KEY = 0;
    uint32_t expected_keys[] = {
        LS(HID_KEY_H), HID_KEY_E, HID_KEY_L, NO_KEY, HID_KEY_L, HID_KEY_O,
        HID_KEY_SPACE,
        LS(HID_KEY_W), HID_KEY_O, HID_KEY_R, HID_KEY_L, HID_KEY_D,
        HID_KEY_1, HID_KEY_2, HID_KEY_3, LS(HID_KEY_3), LS(HID_KEY_4), LS(HID_KEY_5)
    };

    for (uint32_t i = 0; i < ARRAY_SIZE(expected_keys); i++) {
        // Expectations
        if (i == 0) {
            mock().expectOneCall("keyboard_clear_sent_keys");
        }
        if (i == 3) {
            // No call to "keyboard_send_key", since the second L needs a key release
        } else {
            mock().expectOneCall("keyboard_send_key").withParameter("key", expected_keys[i]);
        }

        // Production call
        bool macro_active = macro_update();

        // Checks
        if (i < 3) {
            LONGS_EQUAL(i+1, macro_table[2].send_string.index);
        } else {
            LONGS_EQUAL(i, macro_table[2].send_string.index);
        }
        if (i == ARRAY_SIZE(expected_keys)-1) {
            CHECK_FALSE(macro_active);
            CHECK_FALSE(macro_table[2].active);
        } else {
            CHECK_TEXT(macro_active, std::to_string(i).c_str());
            CHECK(macro_table[2].active);
        }
    }
}

TEST(macro, macro_any_active_reflects_internal_state)
{
    *internals->any_macro_active = true;
    CHECK(macro_any_active());

    *internals->any_macro_active = false;
    CHECK_FALSE(macro_any_active());
}
