#include "ll_iter.h"

// public functions
void ll_iter_init(ll_iter_t* iter, ll_allocator_t* alloc, bool forwards) {
    if (iter == NULL || alloc == NULL) {
        return;
    }

    iter->current_node = NULL;
    iter->current_data = NULL;
    iter->forwards = forwards;
    iter->next = forwards ? alloc->active_head : alloc->active_tail;
}

bool ll_iter_next(ll_iter_t* iter) {
    if (iter == NULL || iter->next == NULL || !iter->next->in_use) {
        return false;
    }

    iter->current_node = iter->next;
    iter->current_data = iter->current_node->data;
    iter->next = (iter->forwards) ? iter->next->next : iter->next->prev;
    return true;
}

bool ll_iter_resume_from(ll_iter_t* iter, ll_node_t* node) {
    if (iter == NULL || node == NULL) {
        return false;
    }

    iter->next = node;
    iter->current_node = NULL;
    iter->current_data = NULL;
    return true;
}
