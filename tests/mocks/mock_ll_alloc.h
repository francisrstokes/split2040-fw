#ifndef MOCK_LL_ALLOC_H
#define MOCK_LL_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ll_alloc.h"

typedef struct StLLAlloc_t {
    void (*lla_init)(ll_allocator_t* alloc, void* data_block, ll_node_t* node_block, uint32_t capacity, uint32_t elem_size);
    ll_node_t* (*lla_alloc_head)(ll_allocator_t* alloc);
    ll_node_t* (*lla_alloc_tail)(ll_allocator_t* alloc);
    void (*lla_free)(ll_allocator_t* alloc, ll_node_t* n);
    void (*lla_free_all)(ll_allocator_t* alloc);
} StLLAlloc_t;

typedef struct LLAllocInternals_t {
    // Private functions
    void (*unlink_node)(ll_node_t** head, ll_node_t** tail, ll_node_t* n);
    void (*insert_head)(ll_node_t** head, ll_node_t** tail, ll_node_t* n);
    void (*insert_tail)(ll_node_t** head, ll_node_t** tail, ll_node_t* n);
} LLAllocInternals_t;

// Mock API
void mock_lla_use_mocks(bool use_mocks);
StLLAlloc_t* mock_lla_get_fn_ptr_struct(void);
const StLLAlloc_t* mock_lla_get_prod_fn_ptr_struct(void);
const StLLAlloc_t* mock_lla_get_mock_fn_ptr_struct(void);
LLAllocInternals_t* mock_lla_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
