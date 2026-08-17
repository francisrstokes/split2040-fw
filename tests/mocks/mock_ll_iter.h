#ifndef MOCK_LL_ITER_H
#define MOCK_LL_ITER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ll_iter.h"

typedef struct StLLIter_t {
    void (*ll_iter_init)(ll_iter_t* iter, ll_allocator_t* alloc, bool forwards);
    bool (*ll_iter_next)(ll_iter_t* iter);
    bool (*ll_iter_resume_from)(ll_iter_t* iter, ll_node_t* node);
} StLLIter_t;

typedef struct LLIterInternals_t {
    int _unused;
} LLIterInternals_t;

// Mock API
void mock_ll_iter_use_mocks(bool use_mocks);
StLLIter_t* mock_ll_iter_get_fn_ptr_struct(void);
const StLLIter_t* mock_ll_iter_get_prod_fn_ptr_struct(void);
const StLLIter_t* mock_ll_iter_get_mock_fn_ptr_struct(void);
LLIterInternals_t* mock_ll_iter_get_internals(void);

#ifdef __cplusplus
}
#endif

#endif
