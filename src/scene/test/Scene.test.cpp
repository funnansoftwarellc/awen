#include <gtest/gtest.h>

#include <awen/flecs.h>

import awen.scene;

using namespace awn::scene;

namespace
{
    struct Fixture
    {
        flecs::world world;

        Fixture()
        {
            world.import<Module>();
        }
    };
}

TEST(Scene, ModuleIsImported)
{
    auto f = Fixture{};
    EXPECT_TRUE(f.world.has<Module>());
}

TEST(Scene, TransformAddsWorldTransform)
{
    auto f = Fixture{};

    const auto entity = f.world.entity().set<Transform>({.x = 5.0F, .y = 6.0F});

    const auto* wt = entity.try_get<WorldTransform>();
    ASSERT_NE(wt, nullptr);
    EXPECT_FLOAT_EQ(wt->x, 0.0F);
    EXPECT_FLOAT_EQ(wt->y, 0.0F);
}

TEST(Scene, RootWorldTransformMatchesLocalTransform)
{
    auto f = Fixture{};

    const auto entity = f.world.entity().set<Transform>({.x = 10.0F, .y = 20.0F});
    f.world.progress();

    const auto* wt = entity.try_get<WorldTransform>();
    ASSERT_NE(wt, nullptr);
    EXPECT_FLOAT_EQ(wt->x, 10.0F);
    EXPECT_FLOAT_EQ(wt->y, 20.0F);
}

TEST(Scene, ChildWorldTransformIncludesParentOffset)
{
    auto f = Fixture{};

    const auto parent = f.world.entity().set<Transform>({.x = 100.0F, .y = 200.0F});
    const auto child = f.world.entity().child_of(parent).set<Transform>({.x = 10.0F, .y = 20.0F});

    f.world.progress();

    const auto* parent_wt = parent.try_get<WorldTransform>();
    ASSERT_NE(parent_wt, nullptr);
    EXPECT_FLOAT_EQ(parent_wt->x, 100.0F);
    EXPECT_FLOAT_EQ(parent_wt->y, 200.0F);

    const auto* child_wt = child.try_get<WorldTransform>();
    ASSERT_NE(child_wt, nullptr);
    EXPECT_FLOAT_EQ(child_wt->x, 110.0F);
    EXPECT_FLOAT_EQ(child_wt->y, 220.0F);
}
