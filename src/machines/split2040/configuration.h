#pragma once

#include "../../keyboard.h"

// Layout
#define XXX  KC_NONE
#define LAYOUT_SPLIT2040(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k40, k41, k42, k43, k44, k45, k46, k47) { \
    {k0,  k1,  k2,  k3,  k4,  k5,  k6,  k7,  k8,  k9,  k10, k11}, \
    {k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23}, \
    {k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35}, \
    {k36, k37, k38, k39, k40, k41, k42, k43, k44, k45, k46, k47} \
}

// Matrix
#define MATRIX_SCAN_INTERVAL_MS     (10)
#define MATRIX_ROWS                 (4)
#define MATRIX_COLS                 (12)
#define MATRIX_SETTLE_ITERATIONS    (25)

// USB
#define USB_VID                     (0x7083)
#define USB_PID                     (0x0002)
#define USB_REPORT_INTERVAL         (10)
#define USB_VENDOR_STRING           "Francis Stokes"
#define USB_PRODUCT_STRING          "split2040"

// Bootmagic
#define BOOTMAGIC_COL               (0)
#define BOOTMAGIC_ROW               (0)

// Layers
#define LAYER_QWERTY                (0)
#define LAYER_LOWER                 (1)
#define LAYER_RAISE                 (2)
#define LAYER_FN                    (3)
#define LAYER_MAX                   (4)

// Combos
#define COMBO_MAX                   (16)
#define COMBO_KEYS_MAX              (4)
#define COMBO_DELAY_MS              (50)
#define COMBO_CANCEL_SUPPRESS_MS    (150)

// Double tap
#define DOUBLE_TAP_DELAY_MS         (200)
#define DOUBLE_TAP_MAX              (8)

// Taphold
#define TAP_HOLD_DELAY_MS           (200)
#define TAP_HOLD_MAX                (8)

// Macros
#define MACRO_MAX                   (8)

// LEDs
#define LEDS_WS2812_PIN             (6)
#define LEDS_MAX                    (4)
#define LEDS_BRIGHTNESS_DELTA       (16)
#define LEDS_INDEX_REMAP(index)     (LEDS_MAX - index - 1)
