#include <gtest/gtest.h>

#include <tuple>
#include <vector>

import awen.scene;

using namespace awn::scene;

TEST(HierarchyPool, RootIsValidOnConstruction)
{
    const auto pool = HierarchyPool{};
    EXPECT_TRUE(pool.root().is_valid());
}

TEST(HierarchyPool, AllocateChildReturnsValidId)
{
    auto pool = HierarchyPool{};
    const auto child = pool.allocate(pool.root());
    EXPECT_TRUE(child.is_valid());
}

TEST(HierarchyPool, AllocatedNodeHasCorrectParent)
{
    auto pool = HierarchyPool{};
    const auto child = pool.allocate(pool.root());
    const auto* node = pool.get(child);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->parent, pool.root());
}

TEST(HierarchyPool, AllocatedNodeHasCorrectLocalZ)
{
    auto pool = HierarchyPool{};
    const auto child = pool.allocate(pool.root(), 5);
    const auto* node = pool.get(child);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->local_z, 5);
}

TEST(HierarchyPool, GetReturnsNullAfterFree)
{
    auto pool = HierarchyPool{};
    const auto child = pool.allocate(pool.root());
    pool.free(child);
    EXPECT_EQ(pool.get(child), nullptr);
}

TEST(HierarchyPool, DepthFirstVisitsNodesInOrder)
{
    // Build a tree:
    //   root
    //     A (z=0)
    //       A1 (z=0)
    //       A2 (z=1)
    //     B (z=1)
    // Expected depth-first order: A, A1, A2, B.
    auto pool = HierarchyPool{};
    const auto a = pool.allocate(pool.root(), 0);
    const auto b = pool.allocate(pool.root(), 1);
    const auto a1 = pool.allocate(a, 0);
    const auto a2 = pool.allocate(a, 1);

    auto visited = std::vector<NodeId>{};
    pool.depth_first([&](NodeId id) { visited.push_back(id); });

    ASSERT_EQ(visited.size(), 4U);
    EXPECT_EQ(visited[0], a);
    EXPECT_EQ(visited[1], a1);
    EXPECT_EQ(visited[2], a2);
    EXPECT_EQ(visited[3], b);
}

TEST(HierarchyPool, SiblingsOrderedByLocalZ)
{
    // Siblings added in reverse z order must still be visited low-to-high.
    auto pool = HierarchyPool{};
    const auto high = pool.allocate(pool.root(), 2);
    const auto low = pool.allocate(pool.root(), 0);
    const auto mid = pool.allocate(pool.root(), 1);

    auto visited = std::vector<NodeId>{};
    pool.depth_first([&](NodeId id) { visited.push_back(id); });

    ASSERT_EQ(visited.size(), 3U);
    EXPECT_EQ(visited[0], low);
    EXPECT_EQ(visited[1], mid);
    EXPECT_EQ(visited[2], high);
}

TEST(HierarchyPool, FreedNodeNotVisited)
{
    auto pool = HierarchyPool{};
    const auto a = pool.allocate(pool.root());
    const auto b = pool.allocate(pool.root());
    pool.free(a);

    auto visited = std::vector<NodeId>{};
    pool.depth_first([&](NodeId id) { visited.push_back(id); });

    ASSERT_EQ(visited.size(), 1U);
    EXPECT_EQ(visited[0], b);
}

TEST(HierarchyPool, FreedNodeDetachedFromSiblingList)
{
    // Freeing the middle sibling must leave the remaining siblings linked correctly.
    auto pool = HierarchyPool{};
    const auto a = pool.allocate(pool.root(), 0);
    const auto b = pool.allocate(pool.root(), 1);
    const auto c = pool.allocate(pool.root(), 2);
    pool.free(b);

    auto visited = std::vector<NodeId>{};
    pool.depth_first([&](NodeId id) { visited.push_back(id); });

    ASSERT_EQ(visited.size(), 2U);
    EXPECT_EQ(visited[0], a);
    EXPECT_EQ(visited[1], c);
}

TEST(HierarchyPool, DepthFirstFromSubtree)
{
    // depth_first_from must visit only the subtree rooted at the given node.
    auto pool = HierarchyPool{};
    const auto a = pool.allocate(pool.root());
    const auto a1 = pool.allocate(a);

    // Allocate a sibling of a to confirm it is not visited by depth_first_from(a, …).
    std::ignore = pool.allocate(pool.root());

    auto visited = std::vector<NodeId>{};
    pool.depth_first_from(a, [&](NodeId id) { visited.push_back(id); });

    ASSERT_EQ(visited.size(), 1U);
    EXPECT_EQ(visited[0], a1);
}
