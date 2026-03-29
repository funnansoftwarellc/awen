#include <gtest/gtest.h>

#include <variant>
#include <vector>

#include <flecs.h>

import awen.scene;
import awen.graphics.draw_list;

using namespace awn::scene;
using namespace awn::graphics;

// ── Scene construction ────────────────────────────────────────────────────────

TEST(Scene, RootEntityIsAlive)
{
    auto scene = Scene{};
    EXPECT_TRUE(scene.root().is_alive());
}

// ── Scene::add_child ──────────────────────────────────────────────────────────

TEST(Scene, AddRectChildReturnsAliveEntity)
{
    auto scene = Scene{};
    const auto rect = scene.add_child<RectNode>(scene.root());
    EXPECT_TRUE(rect.is_alive());
}

TEST(Scene, AddTextChildReturnsAliveEntity)
{
    auto scene = Scene{};
    const auto text = scene.add_child<TextNode>(scene.root());
    EXPECT_TRUE(text.is_alive());
}

TEST(Scene, AddVoidChildReturnsAliveEntity)
{
    auto scene = Scene{};
    const auto group = scene.add_child<void>(scene.root());
    EXPECT_TRUE(group.is_alive());
}

TEST(Scene, AddGrandchildReturnsAliveEntity)
{
    auto scene = Scene{};
    const auto group = scene.add_child<void>(scene.root());
    const auto rect = scene.add_child<RectNode>(group);
    EXPECT_TRUE(rect.is_alive());
}

TEST(Scene, AddChildSetsTransformComponent)
{
    auto scene = Scene{};
    const auto e = scene.add_child<RectNode>(scene.root());
    EXPECT_NE(e.try_get<Transform>(), nullptr);
}

TEST(Scene, AddChildSetsDrawOrderComponent)
{
    auto scene = Scene{};
    const auto e = scene.add_child<RectNode>(scene.root(), 3);
    const auto* order = e.try_get<DrawOrder>();
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->z, 3);
}

TEST(Scene, AddVoidChildHasNoVisualComponent)
{
    auto scene = Scene{};
    const auto group = scene.add_child<void>(scene.root());
    EXPECT_EQ(group.try_get<RectNode>(), nullptr);
    EXPECT_EQ(group.try_get<CircleNode>(), nullptr);
    EXPECT_EQ(group.try_get<TextNode>(), nullptr);
}

// ── DrawList output for RectNode ──────────────────────────────────────────────

TEST(Scene, RectNodeEmitsDrawRect)
{
    auto scene = Scene{};
    scene.add_child<RectNode>(scene.root()).set<RectNode>({.width = 100.0F, .height = 50.0F, .color = Color{255, 0, 0, 255}});

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

TEST(Scene, CircleNodeEmitsDrawCircle)
{
    auto scene = Scene{};
    scene.add_child<CircleNode>(scene.root()).set<CircleNode>({.radius = 20.0F, .color = Color{0, 255, 0, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<DrawCircle>(dl.commands()[0]));

    const auto& cmd = std::get<DrawCircle>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.radius, 20.0F);
    EXPECT_EQ(cmd.color.g, 255);
}

TEST(Scene, TextNodeEmitsDrawText)
{
    auto scene = Scene{};
    scene.add_child<TextNode>(scene.root()).set<TextNode>({.text = "hello", .font_size = 16, .color = Color{255, 255, 255, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<DrawText>(dl.commands()[0]));

    const auto& cmd = std::get<DrawText>(dl.commands()[0]);
    EXPECT_EQ(cmd.text, "hello");
    EXPECT_EQ(cmd.font_size, 16);
}

TEST(Scene, VoidNodeEmitsNoDrawCommand)
{
    auto scene = Scene{};
    [[maybe_unused]] const auto group = scene.add_child<void>(scene.root());

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    EXPECT_EQ(dl.size(), 0U);
}

// ── World-transform propagation ───────────────────────────────────────────────

TEST(Scene, TransformOffsetAppliedToRect)
{
    auto scene = Scene{};
    auto e = scene.add_child<RectNode>(scene.root());
    e.set<Transform>({.x = 100.0F, .y = 200.0F});
    e.set<RectNode>({.width = 10.0F, .height = 10.0F, .color = {}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<DrawRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 100.0F);
    EXPECT_FLOAT_EQ(cmd.y, 200.0F);
}

TEST(Scene, ChildInheritsParentTransform)
{
    // Parent at (100, 200); child at (10, 20) — expected world pos (110, 220).
    auto scene = Scene{};
    auto parent = scene.add_child<RectNode>(scene.root());
    parent.set<Transform>({.x = 100.0F, .y = 200.0F});
    parent.set<RectNode>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto child = scene.add_child<RectNode>(parent);
    child.set<Transform>({.x = 10.0F, .y = 20.0F});
    child.set<RectNode>({.width = 1.0F, .height = 1.0F, .color = {}});

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
    // A void group at (50, 50) with a rect child at (10, 10).
    // Expected world position of rect: (60, 60).
    auto scene = Scene{};
    auto group = scene.add_child<void>(scene.root());
    group.set<Transform>({.x = 50.0F, .y = 50.0F});

    auto child = scene.add_child<RectNode>(group);
    child.set<Transform>({.x = 10.0F, .y = 10.0F});
    child.set<RectNode>({.width = 1.0F, .height = 1.0F, .color = {}});

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
    const auto root = scene.root();

    // Add in reverse z order; draw list must reflect ascending z.
    scene.add_child<RectNode>(root, 2).set<RectNode>({.width = 1.0F, .height = 1.0F, .color = Color{2, 0, 0, 255}});
    scene.add_child<RectNode>(root, 0).set<RectNode>({.width = 1.0F, .height = 1.0F, .color = Color{0, 0, 0, 255}});
    scene.add_child<RectNode>(root, 1).set<RectNode>({.width = 1.0F, .height = 1.0F, .color = Color{1, 0, 0, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 3U);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[0]).color.r, 0);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[1]).color.r, 1);
    EXPECT_EQ(std::get<DrawRect>(dl.commands()[2]).color.r, 2);
}

// ── build_draw_list is additive (does not clear) ──────────────────────────────

TEST(Scene, BuildDrawListAppendsToExistingList)
{
    auto scene = Scene{};
    scene.add_child<RectNode>(scene.root()).set<RectNode>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};

    // First build: 1 command.
    scene.build_draw_list(dl);
    EXPECT_EQ(dl.size(), 1U);

    // Second build without clearing: 2 commands total.
    scene.build_draw_list(dl);
    EXPECT_EQ(dl.size(), 2U);
}
