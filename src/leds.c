#include "leds.h"
#include "color.h"
#include "ws2812.h"

// defines
#define LED_TO_U32(i)       (((uint32_t) (leds_state.leds_out[i][0]) << 8)  | \
                             ((uint32_t) (leds_state.leds_out[i][1]) << 16) | \
                             (uint32_t) (leds_state.leds_out[i][2]))

// statics
static leds_state_t leds_state = {
    .leds = { 0 },
    .leds_out = { 0 },
    .brightness =  64,
    .mask = ((1 << NUM_LEDS) - 1),
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
    }
}

// public functions
void leds_set_color(uint led_index, uint8_t r, uint8_t g, uint8_t b) {
    if (led_index >= NUM_LEDS) return;

    // LED 0 on this board is the rightmost led, so twiddle the index around
    uint adjusted_index = NUM_LEDS - led_index - 1;
    bool led_changed = (leds_state.leds[adjusted_index][0] != r || leds_state.leds[adjusted_index][1] != g || leds_state.leds[adjusted_index][2] != b);

    if (led_changed) {
        leds_state.should_transmit = true;

        leds_state.leds[adjusted_index][0] = r;
        leds_state.leds[adjusted_index][1] = g;
        leds_state.leds[adjusted_index][2] = b;

        leds_compute_brightness_adjusted_color(adjusted_index);

        // If this led is off, or configured off by the user, then set the output directly to black
        if (leds_is_off(adjusted_index)) {
            leds_state.leds_out[adjusted_index][0] = 0;
            leds_state.leds_out[adjusted_index][1] = 0;
            leds_state.leds_out[adjusted_index][2] = 0;
        }
    }
}

void leds_write(void) {
    if (leds_state.should_transmit) {
        for (uint i = 0; i < NUM_LEDS; i++) {
            ws2812_write(LED_TO_U32(i));
        }
    }
    leds_state.should_transmit = false;
}

void leds_init(void) {
    ws2812_init();
}
