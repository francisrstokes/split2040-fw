#include "leds.h"
#include "color.h"
#include "ws2812.h"

#include "pico/stdlib.h"

// defines
#define LED_TO_U32(i)       (((uint32_t) (leds_state.leds_out[i][0]) << 8)  | \
                             ((uint32_t) (leds_state.leds_out[i][1]) << 16) | \
                             (uint32_t) (leds_state.leds_out[i][2]))

// statics
static leds_state_t leds_state = {
    .leds = { 0 },
    .leds_out = { 0 },
    .brightness =  64,
    .mask = ((1 << LEDS_MAX) - 1),
    .should_transmit = false
};

// private functions
static bool leds_is_off(uint index) {
    bool is_actually_off = (leds_state.leds[index][0] == 0) && (leds_state.leds[index][1] == 0) && (leds_state.leds[index][2] == 0);
    bool is_configured_off = (leds_state.mask & (1 << index)) == 0;
    return is_actually_off || is_configured_off;
}

static void leds_compute_brightness_adjusted_color(uint index) {
    if (!leds_is_off(index)) {
        uint8_t color[3] = {0};

        color_rgb2hsl(leds_state.leds[index], color);
        color[2] = leds_state.brightness;
        color_hsl2rgb(color, leds_state.leds_out[index]);
    } else {
        leds_state.leds_out[index][0] = 0;
        leds_state.leds_out[index][1] = 0;
        leds_state.leds_out[index][2] = 0;
    }
}

// public functions
void leds_set_color(uint led_index, uint8_t r, uint8_t g, uint8_t b) {
    if (led_index >= LEDS_MAX) return;

    // LED 0 on this board is the rightmost led, so twiddle the index around
    bool led_changed = (leds_state.leds[led_index][0] != r || leds_state.leds[led_index][1] != g || leds_state.leds[led_index][2] != b);

    if (led_changed) {
        leds_state.should_transmit = true;

        leds_state.leds[led_index][0] = r;
        leds_state.leds[led_index][1] = g;
        leds_state.leds[led_index][2] = b;

        leds_compute_brightness_adjusted_color(led_index);
    }
}

void leds_set_r(uint led_index, uint8_t value) {
    if (led_index >= LEDS_MAX) return;
    leds_set_color(led_index, value, leds_state.leds[led_index][1], leds_state.leds[led_index][2]);
}

void leds_set_g(uint led_index, uint8_t value) {
    if (led_index >= LEDS_MAX) return;
    leds_set_color(led_index, leds_state.leds[led_index][0], value, leds_state.leds[led_index][2]);
}

void leds_set_b(uint led_index, uint8_t value) {
    if (led_index >= LEDS_MAX) return;
    leds_set_color(led_index, leds_state.leds[led_index][0], leds_state.leds[led_index][1], value);
}

void leds_write(void) {
    if (leds_state.should_transmit) {
        for (uint i = 0; i < LEDS_MAX; i++) {
            ws2812_write(LED_TO_U32(LEDS_INDEX_REMAP(i)));
        }
    }
    leds_state.should_transmit = false;
}

void leds_init(void) {
    ws2812_init();
}

void leds_brightness_up(void) {
    if (leds_state.brightness < (0x100 - LEDS_BRIGHTNESS_DELTA)) {
        leds_state.brightness += LEDS_BRIGHTNESS_DELTA;

        for (uint i = 0; i < LEDS_MAX; i++) {
            leds_compute_brightness_adjusted_color(i);
        }

        leds_write();
    }
}

void leds_brightness_down(void) {
    if (leds_state.brightness > 0) {
        leds_state.brightness -= LEDS_BRIGHTNESS_DELTA;

        for (uint i = 0; i < LEDS_MAX; i++) {
            leds_compute_brightness_adjusted_color(i);
        }

        leds_write();
    }
}

void leds_toggle_led_enabled(uint led_index) {
    leds_state.mask ^= (1 << led_index);
    leds_compute_brightness_adjusted_color(led_index);
}
