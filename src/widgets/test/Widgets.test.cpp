#include <gtest/gtest.h>

#include <variant>

#include <awen/flecs.h>

import awen.widgets;

using namespace awn::scene;
using namespace awn::graphics;
using namespace awn::widgets;

namespace
{
    struct Fixture
    {
        flecs::world world;
        awn::widgets::TextureCache textures;

        Fixture()
        {
            world.import<awn::scene::Module>();
            world.import<awn::widgets::Module>();
        }
    };
}

TEST(Widgets, ModuleIsImported)
{
    auto f = Fixture{};
    EXPECT_TRUE(f.world.has<awn::widgets::Module>());
}

TEST(Widgets, CreateWindowCreatesRootEntity)
{
    auto f = Fixture{};

    const auto root = create_window(f.world);
    EXPECT_TRUE(root.is_alive());

    EXPECT_TRUE(root.has<awn::widgets::Window>());
    EXPECT_NE(root.try_get<Transform>(), nullptr);

    const auto* order = root.try_get<DrawOrder>();
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->z, z_layers::ui_base);
}

TEST(Widgets, BuildDrawListEmitsRect)
{
    auto f = Fixture{};

    f.world.entity()
        .set<Transform>({.x = 100.0F, .y = 200.0F})
        .set<DrawOrder>({.z = 0})
        .set<DrawRect>({.width = 30.0F, .height = 40.0F, .color = colors::white});

    auto out = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, out);

    ASSERT_EQ(out.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<RenderRect>(out.commands()[0]));

    const auto& cmd = std::get<RenderRect>(out.commands()[0]);
    EXPECT_FLOAT_EQ(cmd.x, 100.0F);
    EXPECT_FLOAT_EQ(cmd.y, 200.0F);
}

TEST(Widgets, BuildDrawListSortsByAscendingZ)
{
    auto f = Fixture{};

    f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 2}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = Color{2, 0, 0, 255}});

    f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 0}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = Color{0, 0, 0, 255}});

    f.world.entity().set<Transform>({}).set<DrawOrder>({.z = 1}).set<DrawRect>({.width = 1.0F, .height = 1.0F, .color = Color{1, 0, 0, 255}});

    auto out = DrawList{};
    f.world.progress();
    build_draw_list(f.world, f.textures, out);

    ASSERT_EQ(out.size(), 3U);
    EXPECT_EQ(std::get<RenderRect>(out.commands()[0]).color.r, 0);
    EXPECT_EQ(std::get<RenderRect>(out.commands()[1]).color.r, 1);
    EXPECT_EQ(std::get<RenderRect>(out.commands()[2]).color.r, 2);
}

TEST(Widgets, ZLayerBandsAreAscending)
{
    EXPECT_LT(z_layers::game_background, z_layers::game_playfield);
    EXPECT_LT(z_layers::game_playfield, z_layers::game_overlay);
    EXPECT_LT(z_layers::game_overlay, z_layers::ui_base);
}

TEST(Widgets, ConfigureRenderingStoresContext)
{
    auto f = Fixture{};
    const auto clear = Color{.r = 1, .g = 2, .b = 3, .a = 4};

    configure_rendering(f.world, f.textures, clear);

    const auto* context = f.world.try_get<RenderContext>();
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->textures, &f.textures);
    EXPECT_TRUE(context->enabled);
    EXPECT_EQ(context->clear_color.r, clear.r);
    EXPECT_EQ(context->clear_color.g, clear.g);
    EXPECT_EQ(context->clear_color.b, clear.b);
    EXPECT_EQ(context->clear_color.a, clear.a);
}
