#include "ll_alloc.h"

// private functions
static void unlink_node(ll_node_t** head, ll_node_t** tail, ll_node_t* n) {
    if (n->prev) n->prev->next = n->next;
    else* head = n->next;

    if (n->next) n->next->prev = n->prev;
    else* tail = n->prev;

    n->prev = n->next = NULL;
}

static void insert_head(ll_node_t** head, ll_node_t** tail, ll_node_t* n) {
    n->prev = NULL;
    n->next =* head;
    if (*head) {
        (*head)->prev = n;
    } else {
        *tail = n;
    }
    *head = n;
}

static void insert_tail(ll_node_t** head, ll_node_t** tail, ll_node_t* n) {
    n->next = NULL;
    n->prev =* tail;
    if (*tail) {
        (*tail)->next = n;
    } else {
        *head = n;
    }
    *tail = n;
}

// public functions
void lla_init(ll_allocator_t* alloc, void* data_block, ll_node_t* node_block, uint32_t capacity, uint32_t elem_size) {
    alloc->data_block  = data_block;
    alloc->node_block  = node_block;
    alloc->capacity    = capacity;
    alloc->elem_size   = elem_size;
    alloc->active_head = alloc->active_tail = NULL;

    // link nodes into free list
    alloc->free_head = &node_block[0];
    alloc->free_tail = &node_block[capacity - 1];

    for (uint32_t i = 0; i < capacity; i++) {
        ll_node_t* n = &node_block[i];
        n->prev = (i == 0) ? NULL : &node_block[i - 1];
        n->next = (i == capacity - 1) ? NULL : &node_block[i + 1];
        n->data = (uint8_t*)data_block + i*  elem_size;
    }
}

ll_node_t* lla_alloc_head(ll_allocator_t* alloc) {
    ll_node_t* n = alloc->free_head;
    if (!n) return NULL; // no free nodes

    unlink_node(&alloc->free_head, &alloc->free_tail, n);
    insert_head(&alloc->active_head, &alloc->active_tail, n);
    return n;
}

ll_node_t* lla_alloc_tail(ll_allocator_t* alloc) {
    ll_node_t* n = alloc->free_head;
    if (!n) return NULL;

    unlink_node(&alloc->free_head, &alloc->free_tail, n);
    insert_tail(&alloc->active_head, &alloc->active_tail, n);
    return n;
}

void lla_free(ll_allocator_t* alloc, ll_node_t* n) {
    unlink_node(&alloc->active_head, &alloc->active_tail, n);
    insert_head(&alloc->free_head, &alloc->free_tail, n);
}
