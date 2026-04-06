#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <variant>
#include <vector>

import awen.scene;
import awen.graphics.draw_list;

using namespace awen::scene;
using namespace awen::graphics;

// ── NodePool::allocateAt ─────────────────────────────────────────────────────

TEST(NodePoolAllocateAt, ActivatesSlotAtRequestedIndex)
{
    auto pool = NodePool<int>{};
    const auto id = pool.allocateAt(3);
    EXPECT_EQ(id.index, 3U);
    EXPECT_TRUE(id.isValid());
    ASSERT_NE(pool.get(id), nullptr);
}

TEST(NodePoolAllocateAt, GapSlotsReturnNullptr)
{
    auto pool = NodePool<int>{};
    pool.allocateAt(2);

    // Indices 0 and 1 are gap slots (generation 0, never alive).
    EXPECT_EQ(pool.get(NodeId{.index = 0, .generation = 1}), nullptr);
    EXPECT_EQ(pool.get(NodeId{.index = 1, .generation = 1}), nullptr);
}

TEST(NodePoolAllocateAt, SequentialAllocateAfterAllocateAtYieldsNextIndex)
{
    auto pool = NodePool<int>{};
    pool.allocateAt(2);

    // Normal allocate() must grow from size 3, yielding index 3.
    const auto id = pool.allocate();
    EXPECT_EQ(id.index, 3U);
}

// ── Scene construction ────────────────────────────────────────────────────────

TEST(Scene, RootHandleIsValid)
{
    auto scene = Scene{};
    EXPECT_TRUE(scene.root().nodeId().isValid());
}

// ── NodeHandle::addChild ─────────────────────────────────────────────────────

TEST(Scene, AddRectChildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto rect = scene.root().addChild<RectNode>();
    EXPECT_TRUE(rect.nodeId().isValid());
}

TEST(Scene, AddTextChildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto text = scene.root().addChild<TextNode>();
    EXPECT_TRUE(text.nodeId().isValid());
}

TEST(Scene, AddVoidChildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto group = scene.root().addChild<void>();
    EXPECT_TRUE(group.nodeId().isValid());
}

TEST(Scene, AddGrandchildReturnsValidHandle)
{
    auto scene = Scene{};
    const auto group = scene.root().addChild<void>();
    const auto rect = group.addChild<RectNode>();
    EXPECT_TRUE(rect.nodeId().isValid());
}

// ── DrawList output for RectNode ──────────────────────────────────────────────

TEST(Scene, RectNodeEmitsDrawRect)
{
    auto scene = Scene{};
    std::ignore = scene.root().addChild<RectNode>().setSize(100.0F, 50.0F).setColor(Color{255, 0, 0, 255});

    auto dl = DrawList{};
    scene.buildDrawList(dl);

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
    std::ignore = scene.root().addChild<RectNode>().setTransform(Transform{.x = 30.0F, .y = 15.0F}).setSize(10.0F, 10.0F);

    auto dl = DrawList{};
    scene.buildDrawList(dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<DrawRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 30.0F);
    EXPECT_FLOAT_EQ(cmd.y, 15.0F);
}

// ── DrawList output for TextNode ──────────────────────────────────────────────

TEST(Scene, TextNodeEmitsDrawText)
{
    auto scene = Scene{};
    std::ignore = scene.root().addChild<TextNode>().setText("hello").setFontSize(24).setColor(Color{255, 255, 255, 255});

    auto dl = DrawList{};
    scene.buildDrawList(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<DrawText>(dl.commands()[0]));

    const auto& cmd = std::get<DrawText>(dl.commands()[0]);
    EXPECT_EQ(cmd.text, "hello");
    EXPECT_EQ(cmd.fontSize, 24);
}

// ── Void nodes produce no draw commands ───────────────────────────────────────

TEST(Scene, VoidNodeProducesNoDrawCommand)
{
    auto scene = Scene{};
    std::ignore = scene.root().addChild<void>().setTransform(Transform{.x = 100.0F, .y = 50.0F});

    auto dl = DrawList{};
    scene.buildDrawList(dl);

    EXPECT_TRUE(dl.empty());
}

// ── World transform propagation ───────────────────────────────────────────────

TEST(Scene, ChildWorldTransformIsParentPlusLocal)
{
    auto scene = Scene{};
    auto parent = scene.root().addChild<RectNode>().setTransform(Transform{.x = 100.0F, .y = 200.0F}).setSize(1.0F, 1.0F);

    std::ignore = parent.addChild<RectNode>().setTransform(Transform{.x = 10.0F, .y = 20.0F}).setSize(1.0F, 1.0F);

    auto dl = DrawList{};
    scene.buildDrawList(dl);

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
    std::ignore = scene.root()
                      .addChild<void>()
                      .setTransform(Transform{.x = 50.0F, .y = 50.0F})
                      .addChild<RectNode>()
                      .setTransform(Transform{.x = 10.0F, .y = 10.0F})
                      .setSize(1.0F, 1.0F);

    auto dl = DrawList{};
    scene.buildDrawList(dl);

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
    std::ignore = root.addChild<RectNode>(2).setSize(1.0F, 1.0F).setColor(Color{2, 0, 0, 255});
    std::ignore = root.addChild<RectNode>(0).setSize(1.0F, 1.0F).setColor(Color{0, 0, 0, 255});
    std::ignore = root.addChild<RectNode>(1).setSize(1.0F, 1.0F).setColor(Color{1, 0, 0, 255});

    auto dl = DrawList{};
    scene.buildDrawList(dl);

    ASSERT_EQ(dl.size(), 3U);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[0]).color.r, 0);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[1]).color.r, 1);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[2]).color.r, 2);
}

// ── BuildDrawList is additive (does not clear) ─────────────────────────────

TEST(Scene, BuildDrawListAppendsToExistingList)
{
    auto scene = Scene{};
    std::ignore = scene.root().addChild<RectNode>().setSize(1.0F, 1.0F);

    auto dl = DrawList{};

    // First build: 1 command.
    scene.buildDrawList(dl);
    EXPECT_EQ(dl.size(), 1U);

    // Second build without clearing: 2 commands total.
    scene.buildDrawList(dl);
    EXPECT_EQ(dl.size(), 2U);
}
