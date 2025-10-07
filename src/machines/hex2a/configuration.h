#pragma once

#include "../../keyboard.h"

// Layout
#define XXX  KC_NONE
#define LAYOUT_HEX2A(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35, k39, k40, k41, k42, k43, k44) { \
    {k0,  k1,  k2,  k3,  k4,  k5,  k6,  k7,  k8,  k9,  k10, k11}, \
    {k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23}, \
    {k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35}, \
    {XXX, XXX, XXX, k39, k40, k41, k42, k43, k44, XXX, XXX, XXX} \
}

// Matrix
#define MATRIX_SCAN_INTERVAL_MS     (10)
#define MATRIX_ROWS                 (4)
#define MATRIX_COLS                 (12)
#define MATRIX_SETTLE_ITERATIONS    (50)

// USB
#define USB_VID                     (0x7083)
#define USB_PID                     (0x0003)
#define USB_REPORT_INTERVAL         (10)
#define USB_VENDOR_STRING           "Francis Stokes"
#define USB_PRODUCT_STRING          "Hex-2a Split Keyboard"

// Bootmagic
#define BOOTMAGIC_COL               (0)
#define BOOTMAGIC_ROW               (0)

// Layers
#define LAYER_QWERTY                (0)
#define LAYER_LOWER                 (1)
#define LAYER_RAISE                 (2)
#define LAYER_FN                    (3)
#define LAYER_SPLIT                 (4)
#define LAYER_MAX                   (5)

// Combos
#define NUM_COMBO_SLOTS             (16)
#define MAX_KEYS_PER_COMBO          (4)
#define COMBO_DELAY_MS              (50)
#define COMBO_CANCEL_SUPPRESS_MS    (150)

// Double tap
#define DOUBLE_TAP_DELAY_MS         (200)
#define MAX_CONCURRENT_DOUBLE_TAPS  (8)

// Taphold
#define TAP_HOLD_DELAY_MS           (200)
#define MAX_CONCURRENT_TAPHOLDS     (8)

// Macros
#define NUM_MACRO_SLOTS             (8)

// LEDs
#define WS2812_PIN                  (28)
#define NUM_LEDS                    (2)
#define BRIGHTNESS_DELTA            (16)

// The LEDs are wired right to left on this board. Use this macro at the edge (just before shifting out) to keep everything else
// thinking that LED 0 is on the left as expected
#define BODGE_INDEX(index)          (NUM_LEDS - index - 1)
