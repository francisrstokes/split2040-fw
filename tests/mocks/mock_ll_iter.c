#include "mock_ll_iter.h"
#include "CppUTestExt/MockSupport_c.h"

#define ll_iter_init            prod_ll_iter_init
#define ll_iter_next            prod_ll_iter_next
#define ll_iter_resume_from     prod_ll_iter_resume_from

#include "ll_iter.c"

#undef ll_iter_init
#undef ll_iter_next
#undef ll_iter_resume_from

// Mocks
static void mock_ll_iter_init(ll_iter_t* iter, ll_allocator_t* alloc, bool forwards) {
    mock_c()->actualCall("ll_iter_init")
    ->withPointerParameters("iter", (void*)iter)
    ->withPointerParameters("alloc", (void*)alloc)
    ->withBoolParameters("forwards", forwards);
}

static bool mock_ll_iter_next(ll_iter_t* iter) {
    mock_c()->actualCall("ll_iter_next")->withPointerParameters("iter", (void*)iter);
    return mock_c()->returnBoolValueOrDefault(false);
}

static bool mock_ll_iter_resume_from(ll_iter_t* iter, ll_node_t* node) {
    mock_c()->actualCall("ll_iter_resume_from")
    ->withPointerParameters("iter", (void*)iter)
    ->withPointerParameters("node", (void*)node);
    return mock_c()->returnBoolValueOrDefault(false);
}

// Function pointer structs
static const StLLIter_t MockStruct = {
    .ll_iter_init = mock_ll_iter_init,
    .ll_iter_next = mock_ll_iter_next,
    .ll_iter_resume_from = mock_ll_iter_resume_from,
};

static const StLLIter_t ProdStruct = {
    .ll_iter_init = prod_ll_iter_init,
    .ll_iter_next = prod_ll_iter_next,
    .ll_iter_resume_from = prod_ll_iter_resume_from,
};

static StLLIter_t ActiveStruct = MockStruct;

// API
void mock_ll_iter_use_mocks(bool use_mocks) {
    if (use_mocks) {
        ActiveStruct = MockStruct;
    } else {
        ActiveStruct = ProdStruct;
    }
}
StLLIter_t* mock_ll_iter_get_fn_ptr_struct(void) {
    return &ActiveStruct;
}

LLIterInternals_t* mock_ll_iter_get_internals(void) {
    static LLIterInternals_t Internals = {0};
    return &Internals;
}

const StLLIter_t* mock_ll_iter_get_prod_fn_ptr_struct(void) {
    return &ProdStruct;
}

const StLLIter_t* mock_ll_iter_get_mock_fn_ptr_struct(void) {
    return &MockStruct;
}

// Originally named functions that can be diverted to function pointers
void ll_iter_init(ll_iter_t* iter, ll_allocator_t* alloc, bool forwards) {
    return ActiveStruct.ll_iter_init(iter, alloc, forwards);
}
bool ll_iter_next(ll_iter_t* iter) {
    return ActiveStruct.ll_iter_next(iter);
}
bool ll_iter_resume_from(ll_iter_t* iter, ll_node_t* node) {
    return ActiveStruct.ll_iter_resume_from(iter, node);
}
