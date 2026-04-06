#include <gtest/gtest.h>
#include <thread>

import awen.core;

TEST(Engine, Singleton)
{
    // Ensure that only one instance of Engine can be created.
    auto* instance1 = awen::core::Engine::instance();
    EXPECT_EQ(instance1, nullptr);

    {
        awen::core::Engine engine;
        auto* instance2 = awen::core::Engine::instance();
        EXPECT_EQ(instance2, &engine);
    }

    auto* instance3 = awen::core::Engine::instance();
    EXPECT_EQ(instance3, nullptr);
}

TEST(Engine, RunLoop)
{
    awen::core::Engine engine;

    int update_count = 0;
    int fixed_update_count = 0;
    int render_count = 0;

    engine.onUpdate().connect([&](auto) { ++update_count; });
    engine.onFixedUpdate().connect([&](auto) { ++fixed_update_count; });
    engine.onRender().connect([&] { ++render_count; });

    // Run the engine for a short time and then stop it.
    std::thread engine_thread(
        [&engine]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            engine.stop();
        });

    const auto result = engine.run();
    EXPECT_EQ(result, 0);
    EXPECT_GT(update_count, 0);
    EXPECT_GT(fixed_update_count, 0);
    EXPECT_GT(render_count, 0);

    engine_thread.join();
}

TEST(Engine, FixedUpdateLimit)
{
    awen::core::Engine engine;

    auto per_frame_count = 0;
    auto max_per_frame = 0;

    engine.onFixedUpdate().connect([&](auto) { ++per_frame_count; });

    // Record the per-frame fixed update count before each render, then reset.
    // This verifies the spiral-of-death cap holds within any single frame.
    engine.onPreRender().connect(
        [&]
        {
            if (per_frame_count > max_per_frame)
            {
                max_per_frame = per_frame_count;
            }

            per_frame_count = 0;
        });

    std::thread engine_thread(
        [&engine]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            engine.stop();
        });

    const auto result = engine.run();
    EXPECT_EQ(result, 0);
    EXPECT_LE(max_per_frame, engine.fixedUpdateLimit());

    engine_thread.join();
}

TEST(Engine, MultipleInstances)
{
    // Ensure that creating multiple instances of Engine throws an exception.
    awen::core::Engine engine1;

    EXPECT_THROW({ awen::core::Engine engine2; }, std::runtime_error);
}