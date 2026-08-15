#pragma once

#include "../../keyboard.h"

#define ____                        KC_TRANS

#define LOWER                       MO(LAYER_LOWER)
#define RAISE                       MO(LAYER_RAISE)
#define FN                          MO(LAYER_FN)
#define SPLIT                       MO(LAYER_SPLIT)

#define GRV_ESC                     TAP_HOLD(KC_ESC, KC_GRAVE, 0x00)

#define C_LEFT                      LC(KC_LEFT)
#define C_DOWN                      LC(KC_DOWN)
#define C_UP                        LC(KC_UP)
#define C_RIGHT                     LC(KC_RIGHT)

#define S_1                         LG_T(LS(KC_1))
#define S_2                         LA_T(LS(KC_2))
#define S_3                         LS_T(LS(KC_3))
#define S_4                         LC_T(LS(KC_4))
#define S_5                         LS(KC_5)
#define S_6                         LS(KC_6)
#define S_7                         LC_T(LS(KC_7))
#define S_8                         LS_T(LS(KC_8))
#define S_9                         LA_T(LS(KC_9))
#define S_0                         LG_T(LS(KC_0))
#define S_MINUS                     LS(KC_MINUS)

#define SPC_ENT                     DT(KC_SPC, KC_ENTER, 0x0)
#define HOME_PU                     DT(KC_HOME, KC_PU, 0x0)
#define END_PD                      DT(KC_END, KC_PD, 0x0)

#define M_DEREF                     MACRO(0)

#define KBC_COM_RESET_TO_BL         (0x0006)
#define KBC_COM_TOGGLE_SNAKE_MODE   (0x0007)

#define KBC(command)                (ENTRY_TYPE_KBC | command)
#define KBC_RESET_TO_BL             KBC(KBC_COM_RESET_TO_BL)
#define KBC_TOGGLE_SNAKE_MODE       KBC(KBC_COM_TOGGLE_SNAKE_MODE)
#define KBC_INDEX_MASK              (0xffff)

#define BL_RST                      KBC_RESET_TO_BL

#define RUN_BUILD                   LC(LS(KC_B))
#define RUN_TESTS                   LC(LA(KC_T))

#define SNAKE                       KBC_TOGGLE_SNAKE_MODE
