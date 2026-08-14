// mock_ll_alloc.c
#include "mock_ll_alloc.h"
#include "CppUTestExt/MockSupport_c.h"

#define lla_init          prod_lla_init
#define lla_alloc_head    prod_lla_alloc_head
#define lla_alloc_tail    prod_lla_alloc_tail
#define lla_free          prod_lla_free
#define lla_free_all      prod_lla_free_all

#include "ll_alloc.c"

#undef lla_init
#undef lla_alloc_head
#undef lla_alloc_tail
#undef lla_free
#undef lla_free_all

// Mocks
static void mock_lla_init(ll_allocator_t* alloc, void* data_block, ll_node_t* node_block, uint32_t capacity, uint32_t elem_size) {
    mock_c()->actualCall("lla_init")
    ->withPointerParameters("alloc", (void*)alloc)
    ->withPointerParameters("data_block", (void*)data_block)
    ->withPointerParameters("node_block", (void*)node_block)
    ->withUnsignedIntParameters("capacity", capacity)
    ->withUnsignedIntParameters("elem_size", elem_size);
}
static ll_node_t* mock_lla_alloc_head(ll_allocator_t* alloc) {
    mock_c()->actualCall("lla_alloc_head")
    ->withPointerParameters("alloc", (void*)alloc);
    return (ll_node_t*)(mock_c()->returnPointerValueOrDefault(NULL));
}
static ll_node_t* mock_lla_alloc_tail(ll_allocator_t* alloc) {
    mock_c()->actualCall("lla_alloc_tail")
    ->withPointerParameters("alloc", (void*)alloc);
    return (ll_node_t*)(mock_c()->returnPointerValueOrDefault(NULL));
}
static void mock_lla_free(ll_allocator_t* alloc, ll_node_t* n) {
    mock_c()->actualCall("lla_free")
    ->withPointerParameters("alloc", (void*)alloc)
    ->withPointerParameters("n", (void*)n);
}
static void mock_lla_free_all(ll_allocator_t* alloc) {
    mock_c()->actualCall("lla_free_all")
    ->withPointerParameters("alloc", (void*)alloc);
}

// Function pointer structs
static const StLLAlloc_t MockStruct = {
    .lla_init = mock_lla_init,
    .lla_alloc_head = mock_lla_alloc_head,
    .lla_alloc_tail = mock_lla_alloc_tail,
    .lla_free = mock_lla_free,
    .lla_free_all = mock_lla_free_all,
};

static const StLLAlloc_t ProdStruct = {
    .lla_init = prod_lla_init,
    .lla_alloc_head = prod_lla_alloc_head,
    .lla_alloc_tail = prod_lla_alloc_tail,
    .lla_free = prod_lla_free,
    .lla_free_all = prod_lla_free_all,
};

static StLLAlloc_t ActiveStruct = MockStruct;

// API
void mock_lla_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        ActiveStruct = ProdStruct;
    }
}
StLLAlloc_t* mock_lla_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

LLAllocInternals_t* mock_lla_get_internals(void) {
    static LLAllocInternals_t Internals = {
        // Private functions
        .unlink_node = unlink_node,
        .insert_head = insert_head,
        .insert_tail = insert_tail,
    };

    return &Internals;
}

// Originally named functions that can be diverted to function pointers
void lla_init(ll_allocator_t* alloc, void* data_block, ll_node_t* node_block, uint32_t capacity, uint32_t elem_size) {
    return ActiveStruct.lla_init(alloc, data_block, node_block, capacity, elem_size);
}
ll_node_t* lla_alloc_head(ll_allocator_t* alloc) {
    return ActiveStruct.lla_alloc_head(alloc);
}
ll_node_t* lla_alloc_tail(ll_allocator_t* alloc) {
    return ActiveStruct.lla_alloc_tail(alloc);
}
void lla_free(ll_allocator_t* alloc, ll_node_t* n) {
    return ActiveStruct.lla_free(alloc, n);
}
void lla_free_all(ll_allocator_t* alloc) {
    return ActiveStruct.lla_free_all(alloc);
}
