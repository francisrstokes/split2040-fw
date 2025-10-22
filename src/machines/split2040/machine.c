#include "../../keyboard.h"

#ifdef KEYBOARD_SPLIT2040

#include "split2040.h"

#include "../../matrix.h"
#include "../../combo.h"
#include "../../macro.h"
#include "../../color.h"
#include "../../leds.h"

#include "pico/bootrom.h"

// statics
static const uint8_t layer_colors[LAYER_MAX][3] = {
    {WHITE},
    {CYAN},
    {ORANGE},
    {MAGENTA},
};

// extern implementations
uint matrix_cols[MATRIX_COLS] = { 5, 4, 3, 2, 1, 0, 20, 21, 22, 26, 27, 28 };
uint matrix_rows[MATRIX_ROWS] = { 19, 18, 17, 16 };

const keymap_entry_t keymap[LAYER_MAX][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_QWERTY] = LAYOUT_SPLIT2040(
        GRV_ESC,   KC_Q,       KC_W,       KC_E,           KC_R,           KC_T,       /* split */     KC_Y,       KC_U,           KC_I,       KC_O,       KC_P,           KC_BSPC,
        KC_TAB,    LG_T(KC_A), LA_T(KC_S), LS_T(KC_D),     LC_T(KC_F),     KC_G,       /* split */     KC_H,       LC_T(KC_J),     LS_T(KC_K), LA_T(KC_L), LG_T(KC_SCLN),  KC_QUOTE,
        KC_LSFT,   KC_Z,       KC_X,       KC_C,           KC_V,           KC_B,       /* split */     KC_N,       KC_M,           KC_COMMA,   KC_DOT,     KC_SLASH,       KC_ENTER,
        KC_LCTL,   KC_HOME,    KC_LALT,    KC_LGUI,        LOWER,          SPC_ENT,    /* split */     KC_SPC,     RAISE,          END_PD,     HOME_PU,    KC_RSFT,        KC_RCTL
    ),

    [LAYER_LOWER] = LAYOUT_SPLIT2040(
        KC_F1,    KC_F2,      KC_F3,      KC_F4,           KC_F5,          KC_F6,       /* split */     KC_F7,     KC_F8,          KC_F9,      KC_F10,     KC_F11,         ____,
        KC_PTSC,  LG_T(KC_1), LA_T(KC_2), LS_T(KC_3),      LC_T(KC_4),     KC_5,        /* split */     KC_6,      LC_T(KC_7),     LS_T(KC_8), LA_T(KC_9), LG_T(KC_0),     KC_MINUS,
        ____,     C_LEFT,     C_DOWN,     C_UP,            C_RIGHT,        ____,        /* split */     ____,      KC_LEFT,        KC_DOWN,    KC_UP,      KC_RIGHT,       M_DEREF,
        ____,     ____,       ____,       ____,            ____,           ____,        /* split */     ____,       ____,          ____,       ____,       ____,           ____
    ),

    [LAYER_RAISE] = LAYOUT_SPLIT2040(
        ____,      KC_BRKT_L,  KC_BRKT_R,  LS(KC_BRKT_L),  LS(KC_BRKT_R),  ____,       /* split */      ____,       LS(KC_BSLS),   KC_BSLS,    KC_EQ,      LS(KC_EQ),      KC_DEL,
        ____,      S_1,        S_2,        S_3,            S_4,            S_5,        /* split */      S_6,        S_7,           S_8,        S_9,        S_0,            S_MINUS,
        ____,      ____,       ____,       ____,           ____,           ____,       /* split */      ____,       KC_LEFT,       KC_DOWN,    KC_UP,      KC_RIGHT,       ____,
        KC_CAPS,   ____,       ____,       ____,           ____,           ____,       /* split */      ____,       ____,          ____,       ____,       ____,           ____
    ),

    [LAYER_FN] = LAYOUT_SPLIT2040(
        BL_RST,    KC_POWER,   ____,       ____,           ____,           ____,       /* split */     ____,       ____,           KC_BGT_DN,  KC_BGT_UP,  ____,           ____,
        ____,      ____,       ____,       ____,           RUN_BUILD,      ____,       /* split */     ____,       RUN_TESTS,      KC_VOL_DN,  KC_VOL_UP,  KC_MUTE,        ____,
        ____,      TOG_L0,     TOG_L1,     TOG_L2,         TOG_L3,         ____,       /* split */     ____,       ____,           L_B_DN,     L_B_UP,     ____,           ____,
        ____,      ____,       ____,       ____,           ____,           ____,       /* split */     ____,       ____,           ____,       ____,       ____,           ____
    )
};

combo_t combos[COMBO_MAX] = {
    [0]  = COMBO2(KC_E,         KC_R,           LS(KC_9)),       // (
    [1]  = COMBO2(KC_U,         KC_I,           LS(KC_0)),       // )
    [2]  = COMBO2(KC_C,         KC_V,           KC_BRKT_L),      // [
    [3]  = COMBO2(KC_M,         KC_COMMA,       KC_BRKT_R),      // ]
    [4]  = COMBO2(KC_V,         KC_B,           LS(KC_BRKT_L)),  // {
    [5]  = COMBO2(KC_N,         KC_M,           LS(KC_BRKT_R)),  // }
    [6]  = COMBO2(KC_W,         KC_E,           KC_TAB),         // Tab
    [7]  = COMBO2(KC_I,         KC_O,           KC_TAB),         // Tab
    [8]  = COMBO2(KC_Q,         KC_W,           KC_CAPS),        // Caps Lock
    [9]  = COMBO2(LOWER,        RAISE,          FN),             // FN layer
    [10] = COMBO2(KC_P,         KC_BSPC,        M_DEREF),        // "->"
    [11] = COMBO_UNUSED,
    [12] = COMBO_UNUSED,
    [13] = COMBO_UNUSED,
    [14] = COMBO_UNUSED,
    [15] = COMBO_UNUSED,
};

const char arrow_deref[] = "->";

macro_t macros[MACRO_MAX] = {
    [0] = SEND_STRING(arrow_deref, sizeof(arrow_deref)),
    [1] = MACRO_UNUSED,
    [2] = MACRO_UNUSED,
    [3] = MACRO_UNUSED,
    [4] = MACRO_UNUSED,
    [5] = MACRO_UNUSED,
    [6] = MACRO_UNUSED,
    [7] = MACRO_UNUSED
};

// overridden weak functions
void layer_post_set(uint8_t layer) {
    leds_set_color(0, layer_colors[layer][0], layer_colors[layer][1], layer_colors[layer][2]);
}

bool kbc_on_key_press(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_KBC) {
        switch (key & KBC_INDEX_MASK) {
            case KBC_COM_BRIGHTNESS_DOWN:       leds_brightness_down();     matrix_suppress_key_until_release(row, col);    return true;
            case KBC_COM_BRIGHTNESS_UP:         leds_brightness_up();       matrix_suppress_key_until_release(row, col);    return true;
            case KBC_COM_LED0_TOGGLE:           leds_toggle_led_enabled(0); matrix_suppress_key_until_release(row, col);    return true;
            case KBC_COM_LED1_TOGGLE:           leds_toggle_led_enabled(1); matrix_suppress_key_until_release(row, col);    return true;
            case KBC_COM_LED2_TOGGLE:           leds_toggle_led_enabled(2); matrix_suppress_key_until_release(row, col);    return true;
            case KBC_COM_LED3_TOGGLE:           leds_toggle_led_enabled(3); matrix_suppress_key_until_release(row, col);    return true;
            case KBC_COM_RESET_TO_BL:           reset_usb_boot(0, 0);                                                       return true;
        }
    }

    return false;
}

void keyboard_on_led_status_report(uint8_t led_status) {
    leds_set_color(3, LED3_W(led_status), LED3_W(led_status), LED3_W(led_status));
}

void keyboard_on_scan_complete(const uint8_t* hid_report) {
    // Show which modifiers are held on LEDs 1 & 2
    leds_set_color(1, LED1_R(hid_report[0]), LED1_G(hid_report[0]), LED1_B(hid_report[0]));
    leds_set_color(2, LED2_W(hid_report[0]), LED2_W(hid_report[0]), LED2_W(hid_report[0]));
}

#endif
