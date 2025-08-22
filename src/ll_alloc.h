#pragma once

#include "pico/stdlib.h"

// typedefs
typedef struct ll_node_t {
    struct ll_node_t* prev;
    struct ll_node_t* next;
    void* data;          // Pointer into user-supplied data array
} ll_node_t;

typedef struct ll_allocator_t {
    ll_node_t* free_head;   ll_node_t* free_tail;
    ll_node_t* active_head; ll_node_t* active_tail;

    uint32_t elem_size;
    uint32_t capacity;

    // Raw array of user data items
    void* data_block;

    // Raw array of ll_node_t (same length as data_block)
    ll_node_t* node_block;
} ll_allocator_t;

// public functions
void lla_init(ll_allocator_t* alloc, void* data_block, ll_node_t* node_block, uint32_t capacity, uint32_t elem_size);
ll_node_t* lla_alloc_head(ll_allocator_t* alloc);
ll_node_t* lla_alloc_tail(ll_allocator_t* alloc);
void lla_free(ll_allocator_t* alloc, ll_node_t* n);
