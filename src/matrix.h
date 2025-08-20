/**
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "pico/types.h"
#include "hid.h"

// defines
#define MATRIX_ROWS (4)
#define MATRIX_COLS (12)

#define ENTRY_TYPE_MASK (0xff00)
#define ENTRY_TYPE_KC   (0x0000)

#define KC_MASK         (0xff)

// matrix macros
#define KEY(value) (value)

// key definitions
#define KC_NONE     KEY(HID_KEY_NONE)
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

// public functions
void matrix_init(void);
void matrix_scan(uint8_t* keyboard_hid_report);
