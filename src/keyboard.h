/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "hid.h"

// typedefs
typedef uint32_t keymap_entry_t;

/*
 .   31:28   .   27:24   .         23:16         .          15:8         .          7:0          .
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 |   Type    |    Arg4   |          Arg8         |       Modifiers       |        keycode        |
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 .        Byte 3         .        Byte 2         .        Byte 1         .         Byte 0        .

*/

// defines
#define NUM_LAYERS          (4)

#define ENTRY_TYPE_MASK     (0xf0000000)
#define ENTRY_ARG4_MASK     (0x0f000000)
#define ENTRY_ARG4_SHIFT    (24)
#define ENTRY_ARG8_MASK     (0x00ff0000)
#define ENTRY_ARG8_SHIFT    (16)

#define ENTRY_ARG4(entry)   ((entry & ENTRY_ARG4_MASK) >> ENTRY_ARG4_SHIFT)
#define ENTRY_ARG8(entry)   ((entry & ENTRY_ARG8_MASK) >> ENTRY_ARG8_SHIFT)

#define ENTRY_TYPE_KC           (0x00000000)
#define ENTRY_TYPE_LAYER        (0x10000000)
#define ENTRY_TYPE_TAPHOLD      (0x20000000)
#define ENTRY_TYPE_DOUBLE_TAP   (0x30000000)

#define KC_MASK             (0x000000ff)
#define KEY_MODS_MASK       (0x0000ff00)
#define KEY_MODS_SHIFT      (8)

#define KEY_MODS(entry)     ((entry & KEY_MODS_MASK) >> KEY_MODS_SHIFT)

#define LAYER_COM_MO        (0x00 << ENTRY_ARG8_SHIFT)

// matrix macros
#define KEY(kc)             (kc)

#define LC(kc)              KEY((1 << (KEY_MODS_SHIFT + 0)) | kc)
#define LS(kc)              KEY((1 << (KEY_MODS_SHIFT + 1)) | kc)
#define LA(kc)              KEY((1 << (KEY_MODS_SHIFT + 2)) | kc)
#define LG(kc)              KEY((1 << (KEY_MODS_SHIFT + 3)) | kc)

#define LAYER_COM(c, index) (ENTRY_TYPE_LAYER | (c) | (index))
#define MO(index)           LAYER_COM(LAYER_COM_MO, index)

#define TAP_HOLD(tkc, hkc, mods)    (ENTRY_TYPE_TAPHOLD | (hkc << ENTRY_ARG8_SHIFT) | ((mods) << ENTRY_ARG4_SHIFT) | (tkc))
#define MOD_TAP(kc, mods)           TAP_HOLD(kc, KC_NONE, mods)

#define LC_T(kc)                    MOD_TAP(kc, (1 << 0))
#define LS_T(kc)                    MOD_TAP(kc, (1 << 1))
#define LA_T(kc)                    MOD_TAP(kc, (1 << 2))
#define LG_T(kc)                    MOD_TAP(kc, (1 << 3))

#define DOUBLE_TAP(tkc, dkc, dmods) (ENTRY_TYPE_DOUBLE_TAP | (dkc << ENTRY_ARG8_SHIFT) | ((dmods) << ENTRY_ARG4_SHIFT) | (tkc))
#define DT(tkc, dkc, dmods)         DOUBLE_TAP(tkc, dkc, dmods)

// key definitions
#define KC_NONE     KEY(HID_KEY_NONE)
#define KC_TRANS    KEY(0x01)
#define KC_ESC      KEY(HID_KEY_ESCAPE)
#define KC_BSPC     KEY(HID_KEY_BACKSPACE)
#define KC_TAB      KEY(HID_KEY_TAB)
#define KC_SCLN     KEY(HID_KEY_SEMICOLON)
#define KC_QUOTE    KEY(HID_KEY_APOSTROPHE)
#define KC_COMMA    KEY(HID_KEY_COMMA)
#define KC_DOT      KEY(HID_KEY_PERIOD)
#define KC_SLASH    KEY(HID_KEY_SLASH)
#define KC_ENTER    KEY(HID_KEY_ENTER)
#define KC_HOME     KEY(HID_KEY_HOME)
#define KC_END      KEY(HID_KEY_END)
#define KC_SPC      KEY(HID_KEY_SPACE)
#define KC_MINUS    KEY(HID_KEY_MINUS)
#define KC_BRKT_L   KEY(HID_KEY_BRACKET_LEFT)
#define KC_BRKT_R   KEY(HID_KEY_BRACKET_RIGHT)
#define KC_EQ       KEY(HID_KEY_EQUAL)
#define KC_BSLS     KEY(HID_KEY_BACKSLASH)
#define KC_GRAVE    KEY(HID_KEY_GRAVE)
#define KC_CAPS     KEY(HID_KEY_CAPS_LOCK)

#define KC_LCTL     KEY(HID_KEY_CONTROL_LEFT)
#define KC_LSFT     KEY(HID_KEY_SHIFT_LEFT)
#define KC_LALT     KEY(HID_KEY_ALT_LEFT)
#define KC_LGUI     KEY(HID_KEY_GUI_LEFT)
#define KC_RCTL     KEY(HID_KEY_CONTROL_RIGHT)
#define KC_RSFT     KEY(HID_KEY_SHIFT_RIGHT)
#define KC_RALT     KEY(HID_KEY_ALT_RIGHT)
#define KC_RGUI     KEY(HID_KEY_GUI_RIGHT)

#define KC_A        KEY(HID_KEY_A)
#define KC_B        KEY(HID_KEY_B)
#define KC_C        KEY(HID_KEY_C)
#define KC_D        KEY(HID_KEY_D)
#define KC_E        KEY(HID_KEY_E)
#define KC_F        KEY(HID_KEY_F)
#define KC_G        KEY(HID_KEY_G)
#define KC_H        KEY(HID_KEY_H)
#define KC_I        KEY(HID_KEY_I)
#define KC_J        KEY(HID_KEY_J)
#define KC_K        KEY(HID_KEY_K)
#define KC_L        KEY(HID_KEY_L)
#define KC_M        KEY(HID_KEY_M)
#define KC_N        KEY(HID_KEY_N)
#define KC_O        KEY(HID_KEY_O)
#define KC_P        KEY(HID_KEY_P)
#define KC_Q        KEY(HID_KEY_Q)
#define KC_R        KEY(HID_KEY_R)
#define KC_S        KEY(HID_KEY_S)
#define KC_T        KEY(HID_KEY_T)
#define KC_U        KEY(HID_KEY_U)
#define KC_V        KEY(HID_KEY_V)
#define KC_W        KEY(HID_KEY_W)
#define KC_X        KEY(HID_KEY_X)
#define KC_Y        KEY(HID_KEY_Y)
#define KC_Z        KEY(HID_KEY_Z)

#define KC_0        KEY(HID_KEY_0)
#define KC_1        KEY(HID_KEY_1)
#define KC_2        KEY(HID_KEY_2)
#define KC_3        KEY(HID_KEY_3)
#define KC_4        KEY(HID_KEY_4)
#define KC_5        KEY(HID_KEY_5)
#define KC_6        KEY(HID_KEY_6)
#define KC_7        KEY(HID_KEY_7)
#define KC_8        KEY(HID_KEY_8)
#define KC_9        KEY(HID_KEY_9)
#define KC_0        KEY(HID_KEY_0)

#define KC_F1       KEY(HID_KEY_F1)
#define KC_F2       KEY(HID_KEY_F2)
#define KC_F3       KEY(HID_KEY_F3)
#define KC_F4       KEY(HID_KEY_F4)
#define KC_F5       KEY(HID_KEY_F5)
#define KC_F6       KEY(HID_KEY_F6)
#define KC_F7       KEY(HID_KEY_F7)
#define KC_F8       KEY(HID_KEY_F8)
#define KC_F9       KEY(HID_KEY_F9)
#define KC_F10      KEY(HID_KEY_F10)
#define KC_F11      KEY(HID_KEY_F11)
#define KC_F12      KEY(HID_KEY_F12)

#define KC_RIGHT    KEY(HID_KEY_ARROW_RIGHT)
#define KC_LEFT     KEY(HID_KEY_ARROW_LEFT)
#define KC_DOWN     KEY(HID_KEY_ARROW_DOWN)
#define KC_UP       KEY(HID_KEY_ARROW_UP)

// public functions
void keyboard_init(uint8_t* keyboard_hid_report);
bool keyboard_send_key(keymap_entry_t key);
void keyboard_send_modifiers(uint8_t modifiers);
void keyboard_post_scan(void);
keymap_entry_t keyboard_resolve_key(uint row, uint col);
uint8_t keyboard_get_current_layer(void);
