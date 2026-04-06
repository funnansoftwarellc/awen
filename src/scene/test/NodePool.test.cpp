#include <gtest/gtest.h>

#include <vector>

import awen.scene;

using namespace awen::scene;

namespace
{
    struct SimpleData
    {
        int value{};
    };
}

TEST(NodePool, AllocateReturnsValidId)
{
    auto pool = NodePool<SimpleData>{};
    const auto id = pool.allocate();
    EXPECT_TRUE(id.isValid());
}

TEST(NodePool, GetReturnsDataAfterAllocate)
{
    constexpr auto expected_value = 42;
    auto pool = NodePool<SimpleData>{};
    const auto id = pool.allocate();
    auto* data = pool.get(id);
    ASSERT_NE(data, nullptr);
    data->value = expected_value;
    EXPECT_EQ(pool.get(id)->value, expected_value);
}

TEST(NodePool, GetReturnsNullAfterFree)
{
    auto pool = NodePool<SimpleData>{};
    const auto id = pool.allocate();
    pool.free(id);
    EXPECT_EQ(pool.get(id), nullptr);
}

TEST(NodePool, StaleHandleAfterRealloc)
{
    auto pool = NodePool<SimpleData>{};
    const auto old_id = pool.allocate();
    pool.free(old_id);

    // The pool must reuse the freed slot for the next allocation.
    const auto new_id = pool.allocate();

    // The old handle must be stale after the slot is recycled.
    EXPECT_EQ(pool.get(old_id), nullptr);

    // The new handle must be valid.
    EXPECT_NE(pool.get(new_id), nullptr);

    // Both handles must reference the same backing slot.
    EXPECT_EQ(old_id.index, new_id.index);

    // The recycled slot must have a different generation than the freed one.
    EXPECT_NE(old_id.generation, new_id.generation);
}

TEST(NodePool, NullNodeIsInvalid)
{
    auto pool = NodePool<SimpleData>{};
    EXPECT_EQ(pool.get(NullNode), nullptr);
}

TEST(NodePool, FreeTwiceIsNoop)
{
    auto pool = NodePool<SimpleData>{};
    const auto id = pool.allocate();
    pool.free(id);

    // Freeing an already-freed id must not crash or corrupt pool state.
    pool.free(id);
    EXPECT_EQ(pool.get(id), nullptr);
}

TEST(NodePool, MultipleAllocations)
{
    auto pool = NodePool<SimpleData>{};
    constexpr auto count = 100;
    auto ids = std::vector<NodeId>{};
    ids.reserve(count);
    for (auto i = 0; i < count; ++i)
    {
        const auto id = pool.allocate();
        pool.get(id)->value = i;
        ids.push_back(id);
    }
    for (auto i = 0; i < count; ++i)
    {
        EXPECT_EQ(pool.get(ids[i])->value, i);
    }
}

TEST(NodePool, ForEachVisitsOnlyLiveNodes)
{
    auto pool = NodePool<SimpleData>{};
    const auto id0 = pool.allocate();
    const auto id1 = pool.allocate();
    const auto id2 = pool.allocate();
    pool.get(id0)->value = 1;
    pool.get(id1)->value = 2;
    pool.get(id2)->value = 3;

    // Freeing the middle slot leaves id0 and id2 as the only live nodes.
    pool.free(id1);

    auto sum = 0;
    auto count = 0;
    pool.forEach(
        [&](NodeId, SimpleData& d)
        {
            sum += d.value;
            ++count;
        });
    // Only id0 (value 1) and id2 (value 3) must have been visited.
    EXPECT_EQ(count, 2);
    EXPECT_EQ(sum, 4);
}
