#pragma once

#include "ll_alloc.h"

// typedefs
typedef struct ll_iter_t {
    ll_node_t* next;
    ll_node_t* current_node;
    void* current_data;
    bool forwards;
} ll_iter_t;

// public functions
void ll_iter_init(ll_iter_t* iter, ll_allocator_t* alloc, bool forwards);
bool ll_iter_next(ll_iter_t* iter);
bool ll_iter_resume_from(ll_iter_t* iter, ll_node_t* node);
