#include <gtest/gtest.h>

#include <variant>
#include <vector>

#include <awen/flecs.h>

import awen.scene;
import awen.graphics;

using namespace awn::scene;
using namespace awn::graphics;

// ── Scene construction ────────────────────────────────────────────────────────

TEST(Scene, RootEntityIsAlive)
{
    auto scene = Scene{};
    EXPECT_TRUE(scene.root().is_alive());
}

// ── Raw entity creation helpers ──────────────────────────────────────────────

TEST(Scene, AddRectChildReturnsAliveEntity)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto rect = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({});
    EXPECT_TRUE(rect.is_alive());
}

TEST(Scene, AddTextChildReturnsAliveEntity)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto text = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({}).set<DrawText>({});
    EXPECT_TRUE(text.is_alive());
}

TEST(Scene, AddVoidChildReturnsAliveEntity)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto group = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({});
    EXPECT_TRUE(group.is_alive());
}

TEST(Scene, AddGrandchildReturnsAliveEntity)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto group = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({});
    const auto rect = world.entity().child_of(group).set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({});
    EXPECT_TRUE(rect.is_alive());
}

TEST(Scene, AddChildSetsTransformComponent)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto e = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({});
    EXPECT_NE(e.try_get<Transform>(), nullptr);
}

TEST(Scene, AddChildSetsDrawOrderComponent)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto e = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({.z = 3}).set<DrawRect>({});
    const auto* order = e.try_get<DrawOrder>();
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->z, 3);
}

TEST(Scene, AddVoidChildHasNoVisualComponent)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto group = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({});
    EXPECT_EQ(group.try_get<DrawRect>(), nullptr);
    EXPECT_EQ(group.try_get<DrawCircle>(), nullptr);
    EXPECT_EQ(group.try_get<DrawText>(), nullptr);
}

// ── DrawList output for RectNode ──────────────────────────────────────────────

TEST(Scene, RectNodeEmitsDrawRect)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    world.entity()
        .child_of(scene.root())
        .set<Transform>({})
        .set<DrawOrder>({})
        .set<DrawRect>({.width = 100.0F, .height = 50.0F, .color = Color{255, 0, 0, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<RenderRect>(dl.commands()[0]));

    const auto& cmd = std::get<RenderRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.width, 100.0F);
    EXPECT_FLOAT_EQ(cmd.height, 50.0F);
    EXPECT_EQ(cmd.color.r, 255);
    EXPECT_EQ(cmd.color.g, 0);
    EXPECT_EQ(cmd.color.b, 0);
}

TEST(Scene, CircleNodeEmitsDrawCircle)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({}).set<DrawCircle>({.radius = 20.0F, .color = Color{0, 255, 0, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<RenderCircle>(dl.commands()[0]));

    const auto& cmd = std::get<RenderCircle>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.radius, 20.0F);
    EXPECT_EQ(cmd.color.g, 255);
}

TEST(Scene, TextNodeEmitsDrawText)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    world.entity()
        .child_of(scene.root())
        .set<Transform>({})
        .set<DrawOrder>({})
        .set<DrawText>({.text = "hello", .font_size = 16, .color = Color{255, 255, 255, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<RenderText>(dl.commands()[0]));

    const auto& cmd = std::get<RenderText>(dl.commands()[0]);
    EXPECT_EQ(cmd.text, "hello");
    EXPECT_EQ(cmd.font_size, 16);
}

TEST(Scene, VoidNodeEmitsNoDrawCommand)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    [[maybe_unused]] const auto group = world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    EXPECT_EQ(dl.size(), 0U);
}

// ── World-transform propagation ───────────────────────────────────────────────

TEST(Scene, TransformOffsetAppliedToRect)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    [[maybe_unused]] auto e = world.entity()
                                  .child_of(scene.root())
                                  .set<Transform>({.x = 100.0F, .y = 200.0F})
                                  .set<DrawOrder>({})
                                  .set<DrawRect>({.width = 10.0F, .height = 10.0F, .color = {}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<RenderRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 100.0F);
    EXPECT_FLOAT_EQ(cmd.y, 200.0F);
}

TEST(Scene, ChildInheritsParentTransform)
{
    // Parent at (100, 200); child at (10, 20) — expected world pos (110, 220).
    auto scene = Scene{};
    auto& world = scene.raw_world();

    auto parent = world.entity()
                      .child_of(scene.root())
                      .set<Transform>({.x = 100.0F, .y = 200.0F})
                      .set<DrawOrder>({})
                      .set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    [[maybe_unused]] auto child = world.entity()
                                      .child_of(parent)
                                      .set<Transform>({.x = 10.0F, .y = 20.0F})
                                      .set<DrawOrder>({})
                                      .set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 2U);

    const auto& parent_cmd = std::get<RenderRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(parent_cmd.x, 100.0F);
    EXPECT_FLOAT_EQ(parent_cmd.y, 200.0F);

    const auto& child_cmd = std::get<RenderRect>(dl.commands()[1]);
    EXPECT_FLOAT_EQ(child_cmd.x, 110.0F);
    EXPECT_FLOAT_EQ(child_cmd.y, 220.0F);
}

TEST(Scene, VoidGroupOffsetsPropagateToChildren)
{
    // A void group at (50, 50) with a rect child at (10, 10).
    // Expected world position of rect: (60, 60).
    auto scene = Scene{};
    auto& world = scene.raw_world();

    auto group = world.entity().child_of(scene.root()).set<Transform>({.x = 50.0F, .y = 50.0F}).set<DrawOrder>({});

    [[maybe_unused]] auto child = world.entity()
                                      .child_of(group)
                                      .set<Transform>({.x = 10.0F, .y = 10.0F})
                                      .set<DrawOrder>({})
                                      .set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<RenderRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 60.0F);
    EXPECT_FLOAT_EQ(cmd.y, 60.0F);
}

// ── Draw-order: z-ordering of siblings ────────────────────────────────────────

TEST(Scene, SiblingsEmittedInAscendingZOrder)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    const auto root = scene.root();

    // Add in reverse z order; draw list must reflect ascending z.
    world.entity().child_of(root).set<Transform>({}).set<DrawOrder>({.z = 2}).set<DrawRect>(
        {.width = 1.0F, .height = 1.0F, .color = Color{2, 0, 0, 255}});
    world.entity().child_of(root).set<Transform>({}).set<DrawOrder>({.z = 0}).set<DrawRect>(
        {.width = 1.0F, .height = 1.0F, .color = Color{0, 0, 0, 255}});
    world.entity().child_of(root).set<Transform>({}).set<DrawOrder>({.z = 1}).set<DrawRect>(
        {.width = 1.0F, .height = 1.0F, .color = Color{1, 0, 0, 255}});

    auto dl = DrawList{};
    scene.build_draw_list(dl);

    ASSERT_EQ(dl.size(), 3U);
    EXPECT_EQ(std::get<RenderRect>(dl.commands()[0]).color.r, 0);
    EXPECT_EQ(std::get<RenderRect>(dl.commands()[1]).color.r, 1);
    EXPECT_EQ(std::get<RenderRect>(dl.commands()[2]).color.r, 2);
}

// ── build_draw_list is additive (does not clear) ──────────────────────────────

TEST(Scene, BuildDrawListAppendsToExistingList)
{
    auto scene = Scene{};
    auto& world = scene.raw_world();
    world.entity().child_of(scene.root()).set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};

    // First build: 1 command.
    scene.build_draw_list(dl);
    EXPECT_EQ(dl.size(), 1U);

    // Second build without clearing: 2 commands total.
    scene.build_draw_list(dl);
    EXPECT_EQ(dl.size(), 2U);
}
