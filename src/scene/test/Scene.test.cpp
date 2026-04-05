#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

import awen.scene;
import awen.graphics.draw_list;

using namespace awen::scene;
using namespace awen::graphics;

// ── NodePool::allocate_at ─────────────────────────────────────────────────────

TEST(NodePoolAllocateAt, ActivatesSlotAtRequestedIndex)
{
    auto pool = NodePool<int>{};
    const auto id = pool.allocate_at(3);
    EXPECT_EQ(id.index, 3U);
    EXPECT_TRUE(id.is_valid());
    ASSERT_NE(pool.get(id), nullptr);
}

TEST(NodePoolAllocateAt, GapSlotsReturnNullptr)
{
    auto pool = NodePool<int>{};
    pool.allocate_at(2);

    // Indices 0 and 1 are gap slots (generation 0, never alive).
    EXPECT_EQ(pool.get(NodeId{.index = 0, .generation = 1}), nullptr);
    EXPECT_EQ(pool.get(NodeId{.index = 1, .generation = 1}), nullptr);
}

TEST(NodePoolAllocateAt, SequentialAllocateAfterAllocateAtYieldsNextIndex)
{
    auto pool = NodePool<int>{};
    pool.allocate_at(2);

    // Normal allocate() must grow from size 3, yielding index 3.
    const auto id = pool.allocate();
    EXPECT_EQ(id.index, 3U);
}

// ── Scene construction ────────────────────────────────────────────────────────

TEST(Scene, RootHandleIsValid)
{
    auto scene = Scene{};
    EXPECT_TRUE(scene.root().node_id().is_valid());
}

// ── NodeHandle::add_child ─────────────────────────────────────────────────────

TEST(Scene, AddRectChildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto rect = scene.root().add_child<RectNode>();
    EXPECT_TRUE(rect.node_id().is_valid());
}

TEST(Scene, AddTextChildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto text = scene.root().add_child<TextNode>();
    EXPECT_TRUE(text.node_id().is_valid());
}

TEST(Scene, AddVoidChildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto group = scene.root().add_child<void>();
    EXPECT_TRUE(group.node_id().is_valid());
}

TEST(Scene, AddGrandchildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto group = scene.root().add_child<void>();
    const auto rect = group.add_child<RectNode>();
    EXPECT_TRUE(rect.node_id().is_valid());
}

// ── DrawList output for RectNode ──────────────────────────────────────────────

TEST(Scene, RectNodeEmitsDrawRect)
{
    auto scene = Scene{};
    scene.root().add_child<RectNode>().set_size(100.0F, 50.0F).set_color(Color{255, 0, 0, 255});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<DrawRect>(dl.commands()[0]));

    const auto& cmd = std::get<DrawRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.width, 100.0F);
    EXPECT_FLOAT_EQ(cmd.height, 50.0F);
    EXPECT_EQ(cmd.color.r, 255);
    EXPECT_EQ(cmd.color.g, 0);
    EXPECT_EQ(cmd.color.b, 0);
}

TEST(Scene, RectNodePositionFromTransform)
{
    auto scene = Scene{};
    scene.root().add_child<RectNode>().set_transform(Transform{.x = 30.0F, .y = 15.0F}).set_size(10.0F, 10.0F);

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<DrawRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 30.0F);
    EXPECT_FLOAT_EQ(cmd.y, 15.0F);
}

// ── DrawList output for TextNode ──────────────────────────────────────────────

TEST(Scene, TextNodeEmitsDrawText)
{
    auto scene = Scene{};
    scene.root().add_child<TextNode>().set_text("hello").set_font_size(24).set_color(Color{255, 255, 255, 255});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<DrawText>(dl.commands()[0]));

    const auto& cmd = std::get<DrawText>(dl.commands()[0]);
    EXPECT_EQ(cmd.text, "hello");
    EXPECT_EQ(cmd.font_size, 24);
}

// ── Void nodes produce no draw commands ───────────────────────────────────────

TEST(Scene, VoidNodeProducesNoDrawCommand)
{
    auto scene = Scene{};
    scene.root().add_child<void>().set_transform(Transform{.x = 100.0F, .y = 50.0F});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    EXPECT_TRUE(dl.empty());
}

// ── World transform propagation ───────────────────────────────────────────────

TEST(Scene, ChildWorldTransformIsParentPlusLocal)
{
    auto scene = Scene{};
    auto parent = scene.root().add_child<RectNode>().set_transform(Transform{.x = 100.0F, .y = 200.0F}).set_size(1.0F, 1.0F);

    parent.add_child<RectNode>().set_transform(Transform{.x = 10.0F, .y = 20.0F}).set_size(1.0F, 1.0F);

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 2U);

    const auto& parent_cmd = std::get<DrawRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(parent_cmd.x, 100.0F);
    EXPECT_FLOAT_EQ(parent_cmd.y, 200.0F);

    const auto& child_cmd = std::get<DrawRect>(dl.commands()[1]);
    EXPECT_FLOAT_EQ(child_cmd.x, 110.0F);
    EXPECT_FLOAT_EQ(child_cmd.y, 220.0F);
}

TEST(Scene, VoidGroupOffsetsPropagateToChildren)
{
    // A void group node at (50, 50) with a rect child at (10, 10).
    // Expected world position of rect: (60, 60).
    auto scene = Scene{};
    scene.root()
        .add_child<void>()
        .set_transform(Transform{.x = 50.0F, .y = 50.0F})
        .add_child<RectNode>()
        .set_transform(Transform{.x = 10.0F, .y = 10.0F})
        .set_size(1.0F, 1.0F);

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<DrawRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 60.0F);
    EXPECT_FLOAT_EQ(cmd.y, 60.0F);
}

// ── Draw-order: z-ordering of siblings ────────────────────────────────────────

TEST(Scene, SiblingsEmittedInAscendingZOrder)
{
    auto scene = Scene{};
    auto root = scene.root();

    // Add in reverse z order; draw list must reflect ascending z.
    root.add_child<RectNode>(2).set_size(1.0F, 1.0F).set_color(Color{2, 0, 0, 255});
    root.add_child<RectNode>(0).set_size(1.0F, 1.0F).set_color(Color{0, 0, 0, 255});
    root.add_child<RectNode>(1).set_size(1.0F, 1.0F).set_color(Color{1, 0, 0, 255});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 3U);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[0]).color.r, 0);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[1]).color.r, 1);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[2]).color.r, 2);
}

// ── build_draw_list is additive (does not clear) ─────────────────────────────

TEST(Scene, BuildDrawListAppendsToExistingList)
{
    auto scene = Scene{};
    scene.root().add_child<RectNode>().set_size(1.0F, 1.0F);

    auto dl = DrawList{};

    // First build: 1 command.
    scene.build_draw_list(dl);
    EXPECT_EQ(dl.size(), 1U);

    // Second build without clearing: 2 commands total.
    scene.build_draw_list(dl);
    EXPECT_EQ(dl.size(), 2U);
}
