#include "layers.h"
#include "keyboard.h"
#include "matrix.h"
#include "color.h"
#include "leds.h"

// statics
static layer_state_t layer_state = {
    .base = LAYER_QWERTY,
    .current = LAYER_QWERTY,
};

static const uint8_t layer_colors[LAYER_MAX][3] = {
    {WHITE},
    {CYAN},
    {ORANGE},
    {MAGENTA},
    {AQUAMARINE},
};

// public functions
bool layers_on_key_press(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
        if ((key & ENTRY_ARG8_MASK) == LAYER_COM_MO) {
            // A momentary layer switch is only active while the key is pressed
            layers_set(key & KC_MASK);

            // Don't process this entry on further operations
            matrix_mark_key_as_handled(row, col);

            return true;
        }
    }

    return false;
}
bool layers_on_key_release(uint row, uint col, keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
        if ((key & ENTRY_ARG8_MASK) == LAYER_COM_MO) {
            layers_set(layer_state.base);

            // If a momentary layer key is released, ignore active keypresses until they're released
            matrix_suppress_held_until_release();

            return true;
        }
    }

    return false;
}
bool layers_on_virtual_key(keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
        if ((key & ENTRY_ARG8_MASK) == LAYER_COM_MO) {
            layers_set(key & KC_MASK);
            return true;
        }
    }

    return false;
}

uint8_t layers_get_current(void) {
    return layer_state.current;
}

uint8_t layers_get_base(void) {
    return layer_state.base;
}

void layers_set(uint8_t layer) {
    layer_state.current = layer;
    leds_set_color(0, layer_colors[layer][0], layer_colors[layer][1], layer_colors[layer][2]);
}
