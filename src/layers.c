#include "layers.h"
#include "keyboard.h"
#include "matrix.h"

// statics
static layer_state_t layer_state = {0};

// private functions
static bool layers_on_press(uint row, uint col, keymap_entry_t key) {
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

static bool layers_on_release(uint row, uint col, keymap_entry_t key) {
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

static bool layers_on_virtual_press(keymap_entry_t key) {
    if ((key & ENTRY_TYPE_MASK) == ENTRY_TYPE_LAYER) {
        if ((key & ENTRY_ARG8_MASK) == LAYER_COM_MO) {
            layers_set(key & KC_MASK);
            return true;
        }
    }

    return false;
}

static void layers_reset(void) {
    layers_set(layer_state.base);
}

static void layers_init(void* init_data) {
    (void)init_data;
    layers_set(0);
}

static bool layers_update(void) {
    // no-op
    return false;
}

// public functions
const keyboard_system_t* layers_system(void) {
    static const keyboard_system_t system = {
        .name = "layers",
        .init = layers_init,
        .reset = layers_reset,
        .update = layers_update,
        .on_press = layers_on_press,
        .on_virtual_press = layers_on_virtual_press,
        .on_release = layers_on_release,
    };

    return &system;
}

uint8_t layers_get_current(void) {
    return layer_state.current;
}

uint8_t layers_get_base(void) {
    return layer_state.base;
}

void layers_set(uint8_t layer) {
    layer_state.current = layer;
    layer_post_set(layer);
}

__attribute__((weak)) void layer_post_set(uint8_t layer) {

}
