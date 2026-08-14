#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "mock_ll_alloc.h"

TEST_GROUP(ll_alloc) {

    LLAllocInternals_t* internals = mock_lla_get_internals();

    void setup() {
        mock_lla_use_mocks(false);

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_lla_use_mocks(true);
    }
};

TEST(ll_alloc, lla_init_with_zero_capacity_leaves_free_list_empty)
{
    // Setup
    const uint32_t CAPACITY = 0;
    ll_allocator_t alloc = {0};

    // Expectations
    // (none)

    // Production call
    lla_init(&alloc, (void*)NULL, (ll_node_t*)NULL, CAPACITY, sizeof(uint32_t));

    // Checks
    CHECK_EQUAL(CAPACITY, alloc.capacity);
    POINTERS_EQUAL(NULL, alloc.free_head);
    POINTERS_EQUAL(NULL, alloc.free_tail);
    POINTERS_EQUAL(NULL, alloc.active_head);
    POINTERS_EQUAL(NULL, alloc.active_tail);
}

TEST(ll_alloc, lla_init_stores_config_and_links_free_list)
{
    // Setup
    const uint32_t CAPACITY = 4;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};

    // Expectations
    // (none)

    // Production call
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    // Checks
    // Verifies stored config, active list starts empty, and free list head/tail.
    CHECK_EQUAL(CAPACITY, alloc.capacity);
    CHECK_EQUAL(sizeof(uint32_t), alloc.elem_size);
    POINTERS_EQUAL(data_block, alloc.data_block);
    POINTERS_EQUAL(node_block, alloc.node_block);
    POINTERS_EQUAL(NULL, alloc.active_head);
    POINTERS_EQUAL(NULL, alloc.active_tail);
    POINTERS_EQUAL(&node_block[0], alloc.free_head);
    POINTERS_EQUAL(&node_block[CAPACITY - 1], alloc.free_tail);

    for (uint32_t i = 0; i < CAPACITY; i++) {
        POINTERS_EQUAL(&data_block[i], node_block[i].data);

        if (i == 0) {
            POINTERS_EQUAL(NULL, node_block[i].prev);
        } else {
            POINTERS_EQUAL(&node_block[i - 1], node_block[i].prev);
        }

        if (i == CAPACITY - 1) {
            POINTERS_EQUAL(NULL, node_block[i].next);
        } else {
            POINTERS_EQUAL(&node_block[i + 1], node_block[i].next);
        }
    }
}

TEST(ll_alloc, lla_alloc_head_moves_a_node_from_free_to_active)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    // Expectations
    // (none)

    // Production call
    ll_node_t* n = lla_alloc_head(&alloc);

    // Checks
    POINTERS_EQUAL(&node_block[0], n);
    POINTERS_EQUAL(n, alloc.active_head);
    POINTERS_EQUAL(n, alloc.active_tail);
    POINTERS_EQUAL(NULL, n->prev);
    POINTERS_EQUAL(NULL, n->next);
    POINTERS_EQUAL(&node_block[1], alloc.free_head);
}

TEST(ll_alloc, lla_alloc_head_prepends_before_existing_active_head)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_head(&alloc);

    // Expectations
    // (none)

    // Production call
    ll_node_t* n1 = lla_alloc_head(&alloc);

    // Checks
    POINTERS_EQUAL(n1, alloc.active_head);
    POINTERS_EQUAL(n0, alloc.active_tail);
    POINTERS_EQUAL(n0, n1->next);
    POINTERS_EQUAL(n1, n0->prev);
    POINTERS_EQUAL(NULL, n1->prev);
    POINTERS_EQUAL(NULL, n0->next);
}

TEST(ll_alloc, lla_alloc_head_returns_null_when_pool_is_exhausted)
{
    // Setup
    const uint32_t CAPACITY = 1;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));
    lla_alloc_head(&alloc);

    // Expectations
    // (none)

    // Production call
    ll_node_t* n = lla_alloc_head(&alloc);

    // Checks
    POINTERS_EQUAL(NULL, n);
}

TEST(ll_alloc, lla_alloc_tail_moves_a_node_from_free_to_active)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    // Expectations
    // (none)

    // Production call
    ll_node_t* n = lla_alloc_tail(&alloc);

    // Checks
    POINTERS_EQUAL(&node_block[0], n);
    POINTERS_EQUAL(n, alloc.active_head);
    POINTERS_EQUAL(n, alloc.active_tail);
    POINTERS_EQUAL(NULL, n->prev);
    POINTERS_EQUAL(NULL, n->next);
    POINTERS_EQUAL(&node_block[1], alloc.free_head);
}

TEST(ll_alloc, lla_alloc_tail_appends_after_existing_active_tail)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    ll_node_t* n1 = lla_alloc_tail(&alloc);

    // Checks
    POINTERS_EQUAL(n0, alloc.active_head);
    POINTERS_EQUAL(n1, alloc.active_tail);
    POINTERS_EQUAL(n1, n0->next);
    POINTERS_EQUAL(n0, n1->prev);
    POINTERS_EQUAL(NULL, n0->prev);
    POINTERS_EQUAL(NULL, n1->next);
}

TEST(ll_alloc, lla_alloc_tail_returns_null_when_pool_is_exhausted)
{
    // Setup
    const uint32_t CAPACITY = 1;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));
    lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    ll_node_t* n = lla_alloc_tail(&alloc);

    // Checks
    POINTERS_EQUAL(NULL, n);
}

TEST(ll_alloc, lla_free_unlinks_a_middle_active_node_and_returns_it_to_free)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);
    ll_node_t* n1 = lla_alloc_tail(&alloc);
    ll_node_t* n2 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    lla_free(&alloc, n1);

    // Checks
    POINTERS_EQUAL(n0, alloc.active_head);
    POINTERS_EQUAL(n2, alloc.active_tail);
    POINTERS_EQUAL(n2, n0->next);
    POINTERS_EQUAL(n0, n2->prev);
    POINTERS_EQUAL(n1, alloc.free_head);
    POINTERS_EQUAL(n1, alloc.free_tail);
    POINTERS_EQUAL(NULL, n1->prev);
    POINTERS_EQUAL(NULL, n1->next);
}

TEST(ll_alloc, lla_free_unlinks_the_active_head)
{
    // Setup
    const uint32_t CAPACITY = 2;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);
    ll_node_t* n1 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    lla_free(&alloc, n0);

    // Checks
    POINTERS_EQUAL(n1, alloc.active_head);
    POINTERS_EQUAL(n1, alloc.active_tail);
    POINTERS_EQUAL(NULL, n1->prev);
}

TEST(ll_alloc, lla_free_unlinks_the_active_tail)
{
    // Setup
    const uint32_t CAPACITY = 2;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);
    ll_node_t* n1 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    lla_free(&alloc, n1);

    // Checks
    POINTERS_EQUAL(n0, alloc.active_head);
    POINTERS_EQUAL(n0, alloc.active_tail);
    POINTERS_EQUAL(NULL, n0->next);
}

TEST(ll_alloc, lla_free_unlinks_the_only_active_node)
{
    // Setup
    const uint32_t CAPACITY = 1;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    lla_free(&alloc, n0);

    // Checks
    POINTERS_EQUAL(NULL, alloc.active_head);
    POINTERS_EQUAL(NULL, alloc.active_tail);
    POINTERS_EQUAL(n0, alloc.free_head);
    POINTERS_EQUAL(n0, alloc.free_tail);
}

TEST(ll_alloc, lla_free_called_twice_on_the_same_node_is_a_no_op)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);
    ll_node_t* n1 = lla_alloc_tail(&alloc);
    ll_node_t* n2 = lla_alloc_tail(&alloc);

    lla_free(&alloc, n1);

    // Expectations
    // (none)

    // Production call
    lla_free(&alloc, n1);

    // Checks
    POINTERS_EQUAL(n0, alloc.active_head);
    POINTERS_EQUAL(n2, alloc.active_tail);
    POINTERS_EQUAL(n2, n0->next);
    POINTERS_EQUAL(n0, n2->prev);
    POINTERS_EQUAL(n1, alloc.free_head);
    POINTERS_EQUAL(n1, alloc.free_tail);
    POINTERS_EQUAL(NULL, n1->prev);
    POINTERS_EQUAL(NULL, n1->next);
    CHECK_FALSE(n1->in_use);
}

TEST(ll_alloc, lla_free_with_null_node_does_nothing)
{
    // Setup
    const uint32_t CAPACITY = 2;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    lla_free(&alloc, NULL);

    // Checks
    POINTERS_EQUAL(n0, alloc.active_head);
    POINTERS_EQUAL(n0, alloc.active_tail);
}

TEST(ll_alloc, lla_free_all_returns_every_active_node_to_free)
{
    // Setup
    const uint32_t CAPACITY = 3;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    ll_node_t* n0 = lla_alloc_tail(&alloc);
    ll_node_t* n1 = lla_alloc_tail(&alloc);
    ll_node_t* n2 = lla_alloc_tail(&alloc);

    // Expectations
    // (none)

    // Production call
    lla_free_all(&alloc);

    // Checks
    // lla_free_all walks active_head to tail, freeing each node via lla_free, which
    // always inserts at the free list head, so the resulting free list order is the
    // reverse of the original active order.
    POINTERS_EQUAL(NULL, alloc.active_head);
    POINTERS_EQUAL(NULL, alloc.active_tail);
    POINTERS_EQUAL(n2, alloc.free_head);
    POINTERS_EQUAL(n0, alloc.free_tail);
    POINTERS_EQUAL(n1, n2->next);
    POINTERS_EQUAL(n2, n1->prev);
    POINTERS_EQUAL(n0, n1->next);
    POINTERS_EQUAL(n1, n0->prev);
    POINTERS_EQUAL(NULL, n2->prev);
    POINTERS_EQUAL(NULL, n0->next);
}

TEST(ll_alloc, lla_free_all_does_nothing_when_nothing_is_active)
{
    // Setup
    const uint32_t CAPACITY = 2;
    uint32_t data_block[CAPACITY] = {0};
    ll_node_t node_block[CAPACITY] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, CAPACITY, sizeof(uint32_t));

    // Expectations
    // (none)

    // Production call
    lla_free_all(&alloc);

    // Checks
    POINTERS_EQUAL(NULL, alloc.active_head);
    POINTERS_EQUAL(NULL, alloc.active_tail);
    POINTERS_EQUAL(&node_block[0], alloc.free_head);
    POINTERS_EQUAL(&node_block[CAPACITY - 1], alloc.free_tail);
}

TEST(ll_alloc, unlink_node_removes_the_only_node_in_a_list)
{
    // Setup
    ll_node_t n = { .prev = NULL, .next = NULL, .data = NULL };
    ll_node_t* head = &n;
    ll_node_t* tail = &n;

    // Expectations
    // (none)

    // Production call
    internals->unlink_node(&head, &tail, &n);

    // Checks
    POINTERS_EQUAL(NULL, head);
    POINTERS_EQUAL(NULL, tail);
    POINTERS_EQUAL(NULL, n.prev);
    POINTERS_EQUAL(NULL, n.next);
}

TEST(ll_alloc, unlink_node_removes_a_middle_node)
{
    // Setup
    ll_node_t a = { .prev = NULL, .data = NULL };
    ll_node_t b = { .data = NULL };
    ll_node_t c = { .next = NULL, .data = NULL };
    a.next = &b;
    b.prev = &a;
    b.next = &c;
    c.prev = &b;
    ll_node_t* head = &a;
    ll_node_t* tail = &c;

    // Expectations
    // (none)

    // Production call
    internals->unlink_node(&head, &tail, &b);

    // Checks
    POINTERS_EQUAL(&a, head);
    POINTERS_EQUAL(&c, tail);
    POINTERS_EQUAL(&c, a.next);
    POINTERS_EQUAL(&a, c.prev);
    POINTERS_EQUAL(NULL, b.prev);
    POINTERS_EQUAL(NULL, b.next);
}

TEST(ll_alloc, insert_head_into_an_empty_list)
{
    // Setup
    ll_node_t n = { .data = NULL };
    ll_node_t* head = NULL;
    ll_node_t* tail = NULL;

    // Expectations
    // (none)

    // Production call
    internals->insert_head(&head, &tail, &n);

    // Checks
    POINTERS_EQUAL(&n, head);
    POINTERS_EQUAL(&n, tail);
    POINTERS_EQUAL(NULL, n.prev);
    POINTERS_EQUAL(NULL, n.next);
}

TEST(ll_alloc, insert_head_prepends_to_an_existing_list)
{
    // Setup
    ll_node_t a = { .prev = NULL, .next = NULL, .data = NULL };
    ll_node_t b = { .data = NULL };
    ll_node_t* head = &a;
    ll_node_t* tail = &a;

    // Expectations
    // (none)

    // Production call
    internals->insert_head(&head, &tail, &b);

    // Checks
    POINTERS_EQUAL(&b, head);
    POINTERS_EQUAL(&a, tail);
    POINTERS_EQUAL(&a, b.next);
    POINTERS_EQUAL(&b, a.prev);
    POINTERS_EQUAL(NULL, b.prev);
}

TEST(ll_alloc, insert_tail_into_an_empty_list)
{
    // Setup
    ll_node_t n = { .data = NULL };
    ll_node_t* head = NULL;
    ll_node_t* tail = NULL;

    // Expectations
    // (none)

    // Production call
    internals->insert_tail(&head, &tail, &n);

    // Checks
    POINTERS_EQUAL(&n, head);
    POINTERS_EQUAL(&n, tail);
    POINTERS_EQUAL(NULL, n.prev);
    POINTERS_EQUAL(NULL, n.next);
}

TEST(ll_alloc, insert_tail_appends_to_an_existing_list)
{
    // Setup
    ll_node_t a = { .prev = NULL, .next = NULL, .data = NULL };
    ll_node_t b = { .data = NULL };
    ll_node_t* head = &a;
    ll_node_t* tail = &a;

    // Expectations
    // (none)

    // Production call
    internals->insert_tail(&head, &tail, &b);

    // Checks
    POINTERS_EQUAL(&a, head);
    POINTERS_EQUAL(&b, tail);
    POINTERS_EQUAL(&b, a.next);
    POINTERS_EQUAL(&a, b.prev);
    POINTERS_EQUAL(NULL, b.next);
}
