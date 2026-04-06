#include <gtest/gtest.h>

#include <variant>
#include <vector>

import awen.graphics;
import awen.scene;

using namespace awen::graphics;
using namespace awen::scene;

namespace
{
    // Advances pool so that pool.get(target_id) returns a valid pointer, while
    // keeping all intermediate slots dead (nullptr-returning).
    //
    // HierarchyPool allocates its root sentinel at index 0, so the first child
    // has index 1. Fresh component pools always start at index 0, requiring dummy
    // allocations before reaching the target index.  Dummies are freed only after
    // all allocations finish so the free list cannot cause slot reuse mid-loop,
    // which would stall capacity growth.  Dead intermediate slots return nullptr
    // from pool.get(), so the traversal never emits commands for them.
    //
    // @note When calling advance_to multiple times on the same pool, always call in
    //       ascending index order.  Calling with a lower index after a higher one
    //       may free an intermediate slot whose index matches a NodeId already seen,
    //       invalidating a pointer returned by an earlier call.
    template <typename T>
    auto advance_to(NodePool<T>& pool, NodeId target_id) -> T*
    {
        auto to_free = std::vector<NodeId>{};

        while (pool.capacity() <= target_id.index)
        {
            const auto id = pool.allocate();

            if (id.index != target_id.index)
            {
                to_free.push_back(id);
            }
        }

        for (const auto id : to_free)
        {
            pool.free(id);
        }

        return pool.get(target_id);
    }

    // Returns a freshly constructed, empty set of component pools and texture cache.
    struct Pools
    {
        NodePool<Transform> transforms;
        NodePool<RectNode> rects;
        NodePool<CircleNode> circles;
        NodePool<SpriteNode> sprites;
        NodePool<TextNode> texts;
        TextureCache textures;
    };
}

TEST(TraversalPass, EmptyHierarchy_EmitsNoCommands)
{
    const auto hier = HierarchyPool{};
    auto pools = Pools{};
    auto out = DrawList{};

    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    EXPECT_TRUE(out.empty());
}

TEST(TraversalPass, NodeWithNoComponents_EmitsNoCommands)
{
    // A node exists in the hierarchy but has no entries in any component pool.
    auto hier = HierarchyPool{};
    [[maybe_unused]] const auto node = hier.allocate(hier.root());

    auto pools = Pools{};
    auto out = DrawList{};

    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    EXPECT_TRUE(out.empty());
}

TEST(TraversalPass, RectNode_EmitsDrawRectAtNodePosition)
{
    auto hier = HierarchyPool{};
    const auto node = hier.allocate(hier.root());

    auto pools = Pools{};

    auto* transform = advance_to(pools.transforms, node);
    transform->x = 10.0F;
    transform->y = 20.0F;

    auto* rect = advance_to(pools.rects, node);
    rect->width = 50.0F;
    rect->height = 30.0F;
    rect->color = colors::Red;

    auto out = DrawList{};
    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    ASSERT_EQ(out.size(), 1U);
    const auto* cmd = std::get_if<DrawRect>(&out.commands()[0]);
    ASSERT_NE(cmd, nullptr);
    EXPECT_FLOAT_EQ(cmd->x, 10.0F);
    EXPECT_FLOAT_EQ(cmd->y, 20.0F);
    EXPECT_FLOAT_EQ(cmd->width, 50.0F);
    EXPECT_FLOAT_EQ(cmd->height, 30.0F);
}

TEST(TraversalPass, WorldTransform_PropagatesFromParentToChild)
{
    // Parent at (10, 5), child at (3, 2) → child world position must be (13, 7).
    auto hier = HierarchyPool{};
    const auto parent = hier.allocate(hier.root());
    const auto child = hier.allocate(parent);

    auto pools = Pools{};

    auto* parent_transform = advance_to(pools.transforms, parent);
    parent_transform->x = 10.0F;
    parent_transform->y = 5.0F;

    auto* child_transform = advance_to(pools.transforms, child);
    child_transform->x = 3.0F;
    child_transform->y = 2.0F;

    auto* rect = advance_to(pools.rects, child);
    rect->width = 1.0F;
    rect->height = 1.0F;

    auto out = DrawList{};
    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    // Parent emits no command (no RectNode). Child emits one DrawRect at world position.
    ASSERT_EQ(out.size(), 1U);
    const auto* cmd = std::get_if<DrawRect>(&out.commands()[0]);
    ASSERT_NE(cmd, nullptr);
    EXPECT_FLOAT_EQ(cmd->x, 13.0F);
    EXPECT_FLOAT_EQ(cmd->y, 7.0F);
}

TEST(TraversalPass, NodeWithNoTransform_InheritsParentWorldTransform)
{
    // Parent at (8, 4), child has no Transform entry → child world position is (8, 4).
    auto hier = HierarchyPool{};
    const auto parent = hier.allocate(hier.root());
    const auto child = hier.allocate(parent);

    auto pools = Pools{};

    auto* parent_transform = advance_to(pools.transforms, parent);
    parent_transform->x = 8.0F;
    parent_transform->y = 4.0F;

    // No Transform entry for child; advance_to is deliberately not called for child.

    auto* rect = advance_to(pools.rects, child);
    rect->width = 1.0F;
    rect->height = 1.0F;

    auto out = DrawList{};
    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    ASSERT_EQ(out.size(), 1U);
    const auto* cmd = std::get_if<DrawRect>(&out.commands()[0]);
    ASSERT_NE(cmd, nullptr);
    EXPECT_FLOAT_EQ(cmd->x, 8.0F);
    EXPECT_FLOAT_EQ(cmd->y, 4.0F);
}

TEST(TraversalPass, TextNode_EmitsDrawTextAtNodePosition)
{
    auto hier = HierarchyPool{};
    const auto node = hier.allocate(hier.root());

    auto pools = Pools{};

    auto* transform = advance_to(pools.transforms, node);
    transform->x = 5.0F;
    transform->y = 15.0F;

    auto* text = advance_to(pools.texts, node);
    text->text = "Hello";
    text->fontSize = 16;
    text->color = colors::White;

    auto out = DrawList{};
    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    ASSERT_EQ(out.size(), 1U);
    const auto* cmd = std::get_if<DrawText>(&out.commands()[0]);
    ASSERT_NE(cmd, nullptr);
    EXPECT_EQ(cmd->text, "Hello");
    EXPECT_EQ(cmd->x, 5);
    EXPECT_EQ(cmd->y, 15);
    EXPECT_EQ(cmd->fontSize, 16);
}

TEST(TraversalPass, SpriteNode_NullTextureId_EmitsNoCommand)
{
    // SpriteNode with a null TextureId must not produce a DrawSprite command.
    auto hier = HierarchyPool{};
    const auto node = hier.allocate(hier.root());

    auto pools = Pools{};

    auto* sprite = advance_to(pools.sprites, node);
    sprite->textureId = NullTexture;
    sprite->width = 32.0F;
    sprite->height = 32.0F;

    auto out = DrawList{};
    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    EXPECT_TRUE(out.empty());
}

TEST(TraversalPass, CommandOrder_MatchesDepthFirstByZ)
{
    // Build tree:
    //   root
    //     A (z=0)  with RectNode (red)
    //       A1 (z=0) with RectNode (blue)
    //     B (z=1)  with RectNode (green)
    //
    // Depth-first pre-order: A, A1, B → DrawRect(red), DrawRect(blue), DrawRect(green).
    auto hier = HierarchyPool{};
    const auto a = hier.allocate(hier.root(), 0);
    const auto b = hier.allocate(hier.root(), 1);
    const auto a1 = hier.allocate(a, 0);

    auto pools = Pools{};

    // Calls must be in ascending index order (a=1, b=2, a1=3); see advance_to note.
    advance_to(pools.rects, a)->color = colors::Red;
    advance_to(pools.rects, b)->color = colors::Green;
    advance_to(pools.rects, a1)->color = colors::Blue;

    auto out = DrawList{};
    BuildDrawList(hier, pools.transforms, pools.rects, pools.circles, pools.sprites, pools.texts, pools.textures, out);

    ASSERT_EQ(out.size(), 3U);

    const auto* first = std::get_if<DrawRect>(&out.commands()[0]);
    const auto* second = std::get_if<DrawRect>(&out.commands()[1]);
    const auto* third = std::get_if<DrawRect>(&out.commands()[2]);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);

    EXPECT_EQ(first->color, colors::Red);
    EXPECT_EQ(second->color, colors::Blue);
    EXPECT_EQ(third->color, colors::Green);
}
