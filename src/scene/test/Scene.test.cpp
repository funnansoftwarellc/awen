#include <gtest/gtest.h>

#include <variant>
#include <vector>

#include <awen/flecs.h>

import awen.scene;
import awen.graphics;

using namespace awn::scene;
using namespace awn::graphics;

// Helper: a fresh world with SceneModule imported and an empty TextureCache.
namespace
{
    struct Fixture
    {
        flecs::world world;
        TextureCache textures;

        Fixture()
        {
            world.import <SceneModule>();
        }
    };
} // namespace

// ── Module import ─────────────────────────────────────────────────────────────

TEST(Scene, SceneModuleIsImported)
{
    auto f = Fixture{};
    EXPECT_TRUE(f.world.has<SceneModule>());
}

// ── Raw entity creation ──────────────────────────────────────────────────────

TEST(Scene, AddRectEntityReturnsAliveEntity)
{
    auto f = Fixture{};
    const auto rect = f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({});
    EXPECT_TRUE(rect.is_alive());
}

TEST(Scene, AddTextEntityReturnsAliveEntity)
{
    auto f = Fixture{};
    const auto text = f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawText>({});
    EXPECT_TRUE(text.is_alive());
}

TEST(Scene, AddVoidEntityReturnsAliveEntity)
{
    auto f = Fixture{};
    const auto group = f.world.entity().set<Transform>({}).set<DrawOrder>({});
    EXPECT_TRUE(group.is_alive());
}

TEST(Scene, AddChildEntityReturnsAliveEntity)
{
    auto f = Fixture{};
    const auto parent = f.world.entity().set<Transform>({}).set<DrawOrder>({});
    const auto child = f.world.entity().child_of(parent).set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({});
    EXPECT_TRUE(child.is_alive());
}

TEST(Scene, EntityHasTransformComponent)
{
    auto f = Fixture{};
    const auto e = f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({});
    EXPECT_NE(e.try_get<Transform>(), nullptr);
}

TEST(Scene, EntityHasDrawOrderComponent)
{
    auto f = Fixture{};
    const auto e = f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 3}).set<DrawRect>({});
    const auto* order = e.try_get<DrawOrder>();
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->z, 3);
}

TEST(Scene, VoidEntityHasNoVisualComponent)
{
    auto f = Fixture{};
    const auto group = f.world.entity().set<Transform>({}).set<DrawOrder>({});
    EXPECT_EQ(group.try_get<DrawRect>(), nullptr);
    EXPECT_EQ(group.try_get<DrawCircle>(), nullptr);
    EXPECT_EQ(group.try_get<DrawText>(), nullptr);
}

// ── DrawList output ───────────────────────────────────────────────────────────

TEST(Scene, RectNodeEmitsDrawRect)
{
    auto f = Fixture{};
    f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({.width = 100.0F, .height = 50.0F, .color = Color{255, 0, 0, 255}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

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
    auto f = Fixture{};
    f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawCircle>({.radius = 20.0F, .color = Color{0, 255, 0, 255}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<RenderCircle>(dl.commands()[0]));

    const auto& cmd = std::get<RenderCircle>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.radius, 20.0F);
    EXPECT_EQ(cmd.color.g, 255);
}

TEST(Scene, TextNodeEmitsDrawText)
{
    auto f = Fixture{};
    f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawText>({.text = "hello", .font_size = 16, .color = Color{255, 255, 255, 255}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

    ASSERT_EQ(dl.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<RenderText>(dl.commands()[0]));

    const auto& cmd = std::get<RenderText>(dl.commands()[0]);
    EXPECT_EQ(cmd.text, "hello");
    EXPECT_EQ(cmd.font_size, 16);
}

TEST(Scene, VoidNodeEmitsNoDrawCommand)
{
    auto f = Fixture{};
    [[maybe_unused]] const auto group = f.world.entity().set<Transform>({}).set<DrawOrder>({});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

    EXPECT_EQ(dl.size(), 0U);
}

// ── World-transform propagation ───────────────────────────────────────────────

TEST(Scene, TransformOffsetAppliedToRect)
{
    auto f = Fixture{};
    [[maybe_unused]] auto e =
        f.world.entity().set<Transform>({.x = 100.0F, .y = 200.0F}).set<DrawOrder>({}).set<DrawRect>({.width = 10.0F, .height = 10.0F, .color = {}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<RenderRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 100.0F);
    EXPECT_FLOAT_EQ(cmd.y, 200.0F);
}

TEST(Scene, ChildInheritsParentTransform)
{
    // Parent at (100, 200); child at (10, 20) — expected world pos (110, 220).
    auto f = Fixture{};

    auto parent =
        f.world.entity().set<Transform>({.x = 100.0F, .y = 200.0F}).set<DrawOrder>({}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    [[maybe_unused]] auto child = f.world.entity()
                                      .child_of(parent)
                                      .set<Transform>({.x = 10.0F, .y = 20.0F})
                                      .set<DrawOrder>({})
                                      .set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

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
    auto f = Fixture{};

    auto group = f.world.entity().set<Transform>({.x = 50.0F, .y = 50.0F}).set<DrawOrder>({});

    [[maybe_unused]] auto child = f.world.entity()
                                      .child_of(group)
                                      .set<Transform>({.x = 10.0F, .y = 10.0F})
                                      .set<DrawOrder>({})
                                      .set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

    ASSERT_EQ(dl.size(), 1U);
    const auto& cmd = std::get<RenderRect>(dl.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 60.0F);
    EXPECT_FLOAT_EQ(cmd.y, 60.0F);
}

// ── Draw-order: z-ordering of siblings ────────────────────────────────────────

TEST(Scene, SiblingsEmittedInAscendingZOrder)
{
    auto f = Fixture{};

    // Add in reverse z order; draw list must reflect ascending z.
    f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 2}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = Color{2, 0, 0, 255}});
    f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 0}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = Color{0, 0, 0, 255}});
    f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 1}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = Color{1, 0, 0, 255}});

    auto dl = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);

    ASSERT_EQ(dl.size(), 3U);
    EXPECT_EQ(std::get<RenderRect>(dl.commands()[0]).color.r, 0);
    EXPECT_EQ(std::get<RenderRect>(dl.commands()[1]).color.r, 1);
    EXPECT_EQ(std::get<RenderRect>(dl.commands()[2]).color.r, 2);
}

// ── build_draw_list is additive (does not clear) ──────────────────────────────

TEST(Scene, BuildDrawListAppendsToExistingList)
{
    auto f = Fixture{};
    f.world.entity().set<Transform>({}).set<DrawOrder>({}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = {}});

    auto dl = DrawList{};

    // First build: 1 command.
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);
    EXPECT_EQ(dl.size(), 1U);

    // Second build without clearing: 2 commands total.
    f.world.progress();
    build_draw_list(f.world, f.textures, dl);
    EXPECT_EQ(dl.size(), 2U);
}
