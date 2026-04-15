// This header overrides the production one

#define MATRIX_SCAN_INTERVAL_MS     (5)
#define MATRIX_ROWS                 (4)
#define MATRIX_COLS                 (12)
#define MATRIX_SETTLE_ITERATIONS    (50)

// USB
#define USB_VID                     (0xdead)
#define USB_PID                     (0xbeef)
#define USB_REPORT_INTERVAL         MATRIX_SCAN_INTERVAL_MS
#define USB_VENDOR_STRING           "Keyboard Firmware Tests"
#define USB_PRODUCT_STRING          "Test Keyboard"

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

// LEDs
#define LEDS_WS2812_PIN             (28)
#define LEDS_MAX                    (2)
#define LEDS_BRIGHTNESS_DELTA       (16)
#define LEDS_INDEX_REMAP(index)     (LEDS_MAX - index - 1)
#define LEDS_HAS_DEBUG_LED          (1)
#define LEDS_DEBUG_LED_PIN          (25)
