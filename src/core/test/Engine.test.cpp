#include <awen/test/Test.hpp>

import awen.core.engine;

TEST(Engine, Constructor)
{
    const awen::core::Engine engine;
    EXPECT_TRUE(true);
}

UNIT_TEST(Engine, UpdateFixedLimit)
{
    awen::core::Engine engine;

    constexpr auto expectedLimit = 5;
    engine.setUpdateFixedLimit(expectedLimit);
    EXPECT_EQ(engine.getUpdateFixedLimit(), expectedLimit);
}

UNIT_TEST(Engine, UpdateFixedInterval)
{
    awen::core::Engine engine;

    constexpr auto interval = std::chrono::milliseconds(16);
    engine.setUpdateFixedInterval(interval);
    EXPECT_EQ(engine.getUpdateFixedInterval(), interval);
}

UNIT_TEST(Engine, Stop)
{
    awen::core::Engine engine;

    auto connection = engine.onUpdate([&engine](auto) { engine.stop(); });

    EXPECT_EQ(engine.run(), EXIT_SUCCESS);
}
