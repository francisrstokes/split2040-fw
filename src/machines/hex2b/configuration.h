#pragma once

#include "hardware/i2c.h"

#include "../../keyboard.h"

// Layout
#define XXX  KC_NONE
#define LAYOUT_HEX2B(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35, k39, k40, k41, k42, k43, k44) { \
    {k0,  k1,  k2,  k3,  k4,  k5,  k6,  k7,  k8,  k9,  k10, k11}, \
    {k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23}, \
    {k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35}, \
    {XXX, XXX, XXX, k39, k40, k41, k42, k43, k44, XXX, XXX, XXX} \
}

// Matrix
#define MATRIX_SCAN_INTERVAL_MS     (5)
#define MATRIX_ROWS                 (4)
#define MATRIX_COLS                 (12)
#define MATRIX_SETTLE_ITERATIONS    (50)

// USB
#define USB_VID                     (0x7083)
#define USB_PID                     (0x0004)
#define USB_REPORT_INTERVAL         MATRIX_SCAN_INTERVAL_MS
#define USB_VENDOR_STRING           "Francis Stokes"
#define USB_PRODUCT_STRING          "Hex-2b Split Keyboard"

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
#define MACRO_SIZE_MAX              (32)

// Split
// (build defines SPLIT_CONTROLLER and SPLIT_TARGET to indicate which half manages the bus)
#define SPLIT_ENABLE
#define SPLIT_PIO_UART              pio0
#define SPLIT_UART_BAUDRATE         (115200)
#define SPLIT_UART_CONTROLLER_TX    (10)
#define SPLIT_UART_CONTROLLER_RX    (11)
#define SPLIT_UART_TARGET_TX        (11)
#define SPLIT_UART_TARGET_RX        (10)
#define SPLIT_COLS                  (MATRIX_COLS / 2)
#define SPLIT_COL_SHIFT_CONTROLLER  (0)
#define SPLIT_COL_SHIFT_TARGET      (6)