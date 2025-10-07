#pragma once

#include "../../keyboard.h"

#define ____                    KC_TRANS

#define LOWER                   MO(LAYER_LOWER)
#define RAISE                   MO(LAYER_RAISE)
#define FN                      MO(LAYER_FN)
#define SPLIT                   MO(LAYER_SPLIT)

#define GRV_ESC                 TAP_HOLD(KC_ESC, KC_GRAVE, 0x00)

#define C_LEFT                  LC(KC_LEFT)
#define C_DOWN                  LC(KC_DOWN)
#define C_UP                    LC(KC_UP)
#define C_RIGHT                 LC(KC_RIGHT)

#define S_1                     LG_T(LS(KC_1))
#define S_2                     LA_T(LS(KC_2))
#define S_3                     LS_T(LS(KC_3))
#define S_4                     LC_T(LS(KC_4))
#define S_5                     LS(KC_5)
#define S_6                     LS(KC_6)
#define S_7                     LC_T(LS(KC_7))
#define S_8                     LS_T(LS(KC_8))
#define S_9                     LA_T(LS(KC_9))
#define S_0                     LG_T(LS(KC_0))
#define S_MINUS                 LS(KC_MINUS)

#define SPC_ENT                 DT(KC_SPC, KC_ENTER, 0x0)
#define HOME_PU                 DT(KC_HOME, KC_PU, 0x0)
#define END_PD                  DT(KC_END, KC_PD, 0x0)

#define M_DEREF                 MACRO(0)

#define KBC_COM_BRIGHTNESS_UP       (0x0000)
#define KBC_COM_BRIGHTNESS_DOWN     (0x0001)
#define KBC_COM_LED0_TOGGLE         (0x0002)
#define KBC_COM_LED1_TOGGLE         (0x0003)
#define KBC_COM_LED2_TOGGLE         (0x0004)
#define KBC_COM_LED3_TOGGLE         (0x0005)
#define KBC_COM_RESET_TO_BL         (0x0006)
#define KBC_COM_TOGGLE_SNAKE_MODE   (0x0007)

#define KBC(command)                (ENTRY_TYPE_KBC | command)
#define KBC_BRIGHTNESS_UP           KBC(KBC_COM_BRIGHTNESS_UP)
#define KBC_BRIGHTNESS_DOWN         KBC(KBC_COM_BRIGHTNESS_DOWN)
#define KBC_LED0_TOGGLE             KBC(KBC_COM_LED0_TOGGLE)
#define KBC_LED1_TOGGLE             KBC(KBC_COM_LED1_TOGGLE)
#define KBC_LED2_TOGGLE             KBC(KBC_COM_LED2_TOGGLE)
#define KBC_LED3_TOGGLE             KBC(KBC_COM_LED3_TOGGLE)
#define KBC_RESET_TO_BL             KBC(KBC_COM_RESET_TO_BL)
#define KBC_TOGGLE_SNAKE_MODE       KBC(KBC_COM_TOGGLE_SNAKE_MODE)
#define KBC_INDEX_MASK              (0xffff)

#define BL_RST                  KBC_RESET_TO_BL
#define TOG_L0                  KBC_LED0_TOGGLE
#define TOG_L1                  KBC_LED1_TOGGLE
#define TOG_L2                  KBC_LED2_TOGGLE
#define TOG_L3                  KBC_LED3_TOGGLE
#define L_B_UP                  KBC_BRIGHTNESS_UP
#define L_B_DN                  KBC_BRIGHTNESS_DOWN

#define LED1_R(mods)            ((mods & (LA_BIT | RA_BIT)) ? 255 : 0)
#define LED1_G(mods)            ((mods & (LS_BIT | RS_BIT)) ? 255 : 0)
#define LED1_B(mods)            ((mods & (LC_BIT | RC_BIT)) ? 255 : 0)
#define LED2_W(mods)            ((mods & (LG_BIT | RG_BIT)) ? 255 : 0)
#define LED3_W(led_status)      ((led_status & (1 << 1)) ? 255 : 0)
#define SNAKE_LED(enabled)      ((enabled) ? 255 : 0)

#define RUN_BUILD               LC(LS(KC_B))
#define RUN_TESTS               LC(LA(KC_T))

#define SNAKE                   KBC_TOGGLE_SNAKE_MODE
