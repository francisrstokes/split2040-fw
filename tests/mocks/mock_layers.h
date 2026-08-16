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
#include "mock_keyboard_system.h"

typedef struct StLayers_t {
    KB_SYSTEM_FN_PTR(layers);
    uint8_t (*layers_get_current)(void);
    uint8_t (*layers_get_base)(void);
    void (*layers_set)(uint8_t layer);
} StLayers_t;

typedef struct LayersInternals_t {
    layer_state_t* layer_state;

    // private functions
    KB_SYSTEM_INTERNALS_PRIVATE_FNS(layers)
} LayersInternals_t;

// Mock API
void mock_layers_use_mocks(bool use_mocks);
StLayers_t* mock_layers_get_fn_ptr_struct(void);
LayersInternals_t* mock_layers_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
