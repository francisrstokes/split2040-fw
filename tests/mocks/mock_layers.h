#ifndef MOCK_LAYERS_H
#define MOCK_LAYERS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "machines/machine.h"
#include "layers.h"

typedef struct StLayers_t {
    void (*layers_reset)(void);
    bool (*layers_on_key_press)(uint row, uint col, keymap_entry_t key);
    bool (*layers_on_key_release)(uint row, uint col, keymap_entry_t key);
    bool (*layers_on_virtual_key)(keymap_entry_t key);
    uint8_t (*layers_get_current)(void);
    uint8_t (*layers_get_base)(void);
    void (*layers_set)(uint8_t layer);
} StLayers_t;

typedef struct LayersInternals_t {
    layer_state_t* layer_state;
} LayersInternals_t;

// Mock API
void mock_layers_use_mocks(bool use_mocks);
StLayers_t* mock_layers_get_fn_ptr_struct(void);
LayersInternals_t* mock_layers_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
