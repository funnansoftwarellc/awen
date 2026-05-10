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
