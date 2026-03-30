#include <cstdlib>

#include <awen/flecs.h>

import awen.core.ecs;
import awen.graphics.ecs;

namespace
{
    constexpr auto init_width = 1280;
    constexpr auto init_height = 720;
    constexpr auto target_fps = 60;
}

auto main() -> int
try
{
    flecs::world world;

    world.import<awn::core::Module>();
    world.import<awn::graphics::ecs::Module>();

    // Create and configure the window entity.
    auto window = world.entity("MainWindow")
                      .set<awn::graphics::ecs::Window>({
                          .title = "Awen - Pong",
                          .width = init_width,
                          .height = init_height,
                          .flags = {awn::graphics::ecs::Window::ConfigFlag::resizable, awn::graphics::ecs::Window::ConfigFlag::high_dpi},
                      });

    auto running = true;

    // Main loop: advance flecs world each frame.
    while (running)
    {
        world.progress();

        if (const auto* runtime = window.try_get<awn::graphics::ecs::WindowRuntime>(); runtime != nullptr)
        {
            running = !runtime->should_close;
        }
    }

    // world.import<awn::core::Module>();
    // world.import<awn::graphics::Module>();

    // auto window = world.entity("MainWindow");

    // window.set<awn::graphics::WindowComponent>({
    //     .title = "Awen - Pong",
    //     .width = init_width,
    //     .height = init_height,
    //     .flags = {awn::graphics::ConfigFlag::resizable, awn::graphics::ConfigFlag::high_dpi},
    // });

    // window.set<awn::graphics::TargetFPS>({.value = target_fps});

    // auto keep_running = true;

    // auto engine = awn::core::Engine{world};

    // auto window = std::make_unique<awn::widgets::Window>(engine, "Awen - Pong", init_width, init_height, {awn::graphics::ConfigFlag::resizable,
    //      awn::graphics::ConfigFlag::high_dpi});
    //

    // onReady
    // --- Engine Loop ---
    // - Poll events
    // - Update game state
    // - Update physics
    // - PreRender
    // - Render
    // - PostRender

    // while (keep_running)
    // {
    //     world.progress();

    //     if (const auto* runtime = window.try_get<awn::graphics::WindowRuntime>(); runtime != nullptr)
    //     {
    //         keep_running = !runtime->should_close;
    //     }
    // }

    // window.remove<awn::graphics::WindowComponent>();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
