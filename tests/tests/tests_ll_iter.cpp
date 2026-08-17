#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "mock_ll_alloc.h"
#include "mock_ll_iter.h"

#include <vector>
#include <algorithm>

TEST_GROUP(ll_iter) {

    LLIterInternals_t* internals = mock_ll_iter_get_internals();

    void setup() {
        mock_ll_iter_use_mocks(false);
        mock_lla_use_mocks(false);

        mock().strictOrder();
    }

    void teardown() {
        mock().checkExpectations();
        mock().clear();

        mock_ll_iter_use_mocks(true);
        mock_lla_use_mocks(true);
    }

    void setup_list_from_vector(std::vector<uint32_t>& items, ll_allocator_t* alloc) {
        for (auto item : items) {
            ll_node_t* node = lla_alloc_head(alloc);
            if (node == NULL) {
                FAIL("setup_list_from_vector: Not enough space to insert all items.");
            }
            *(uint32_t*)node->data = item;
        }
    }
};

TEST(ll_iter, ll_iter_init_null_iter_early_returns)
{
    // Setup
    ll_allocator_t alloc = {0};

    // Expectations
    // (none)

    // Production call
    ll_iter_init((ll_iter_t*)NULL, &alloc, true);

    // Checks
    // (none)
}

TEST(ll_iter, ll_iter_init_null_allocactor_early_returns)
{
    // Setup
    ll_iter_t iter = {0};

    // Expectations
    // (none)

    // Production call
    ll_iter_init(&iter, (ll_allocator_t*)NULL, true);

    // Checks
    // (none)
}

TEST(ll_iter, ll_iter_init_sets_up_struct_forwards)
{
    // Setup
    ll_node_t node_block[2] = {0};
    uint32_t data_block[2] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, 2, sizeof(uint32_t));

    std::vector<uint32_t> test_data{1, 2};
    setup_list_from_vector(test_data, &alloc);

    ll_iter_t iter = {0};

    // Expectations
    // (none)

    // Production call
    ll_iter_init(&iter, &alloc, true);

    // Checks
    CHECK_EQUAL((ll_node_t*)NULL, iter.current_node);
    CHECK_EQUAL((void*)NULL, iter.current_data);
    CHECK_EQUAL(alloc.active_head, iter.next);
    CHECK_EQUAL(true, iter.forwards);
}

TEST(ll_iter, ll_iter_init_sets_up_struct_backwards)
{
    // Setup
    ll_node_t node_block[2] = {0};
    uint32_t data_block[2] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, 2, sizeof(uint32_t));

    std::vector<uint32_t> test_data{1, 2};
    setup_list_from_vector(test_data, &alloc);

    ll_iter_t iter = {0};

    // Expectations
    // (none)

    // Production call
    ll_iter_init(&iter, &alloc, false);

    // Checks
    CHECK_EQUAL((ll_node_t*)NULL, iter.current_node);
    CHECK_EQUAL((void*)NULL, iter.current_data);
    CHECK_EQUAL(alloc.active_tail, iter.next);
    CHECK_EQUAL(false, iter.forwards);
}

TEST(ll_iter, ll_iter_next_false_on_null_iterator)
{
    // Setup
    // (none)

    // Expectations
    // (none)

    // Production call
    bool result = ll_iter_next((ll_iter_t*)NULL);

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(ll_iter, ll_iter_next_false_when_pointed_node_is_not_active)
{
    // Setup
    ll_node_t node = {
        .prev = (ll_node_t*)1234,
        .next = (ll_node_t*)5678,
        .data = (void*)91011,
        .in_use = false,
    };
    ll_iter_t iter = {
        .next = &node,
        .current_node = (ll_node_t*)NULL,
        .current_data = (void*)NULL,
        .forwards = true
    };

    // Expectations
    // (none)

    // Production call
    bool result = ll_iter_next(&iter);

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(ll_iter, ll_iter_next_iterates_the_list_forward)
{
    // Setup
    ll_node_t node_block[3] = {0};
    uint32_t data_block[3] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, 3, sizeof(uint32_t));

    std::vector<uint32_t> test_data{1, 2, 3};
    std::vector<uint32_t> data_out{};
    setup_list_from_vector(test_data, &alloc);

    ll_iter_t iter = {0};
    ll_iter_init(&iter, &alloc, true);

    // Expectations
    // (none)

    // Production call
    while (!ll_iter_next(&iter)) {
        data_out.push_back(*(uint32_t*)iter.current_data);
    }

    // Checks
    CHECK_EQUAL(true, std::equal(data_out.begin(), data_out.end(), test_data.begin()));
}

TEST(ll_iter, ll_iter_next_iterates_the_list_backward)
{
    // Setup
    ll_node_t node_block[3] = {0};
    uint32_t data_block[3] = {0};
    ll_allocator_t alloc = {0};
    lla_init(&alloc, data_block, node_block, 3, sizeof(uint32_t));

    std::vector<uint32_t> test_data{1, 2, 3};
    std::vector<uint32_t> data_out{};
    setup_list_from_vector(test_data, &alloc);

    ll_iter_t iter = {0};
    ll_iter_init(&iter, &alloc, false);

    // Expectations
    // (none)

    // Production call
    while (!ll_iter_next(&iter)) {
        data_out.push_back(*(uint32_t*)iter.current_data);
    }

    // Reverse the collected vector so it can be directly compared to the setup data
    std::reverse(data_out.begin(), data_out.end());

    // Checks
    CHECK_EQUAL(true, std::equal(data_out.begin(), data_out.end(), test_data.begin()));
}

TEST(ll_iter, ll_iter_resume_from_false_on_null_iterator)
{
    // Setup
    ll_node_t node = {
        .prev = (ll_node_t*)1234,
        .next = (ll_node_t*)5678,
        .data = (void*)91011,
        .in_use = true,
    };

    // Expectations
    // (none)

    // Production call
    bool result = ll_iter_resume_from((ll_iter_t*)NULL, &node);

    // Checks
    CHECK_EQUAL(false, result);
}


TEST(ll_iter, ll_iter_resume_from_false_on_null_node)
{
    // Setup
    ll_iter_t iter = {0};

    // Expectations
    // (none)

    // Production call
    bool result = ll_iter_resume_from(&iter, (ll_node_t*)NULL);

    // Checks
    CHECK_EQUAL(false, result);
}

TEST(ll_iter, ll_iter_resume_from_points_to_provided_node)
{
    // Setup
    ll_node_t node = {0};
    ll_iter_t iter = {.forwards = true};

    // Expectations
    // (none)

    // Production call
    bool result = ll_iter_resume_from(&iter, &node);

    // Checks
    CHECK_EQUAL(true, result);
    CHECK_EQUAL(&node, iter.next);
}
