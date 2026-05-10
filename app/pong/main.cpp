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
    awn::core::Engine engine;
    engine.load<awn::graphics::ecs::Module>();

    // Create and configure the window entity.
    auto window = engine.world()
                      .entity("MainWindow")
                      .set<awn::graphics::ecs::Window>({
                          .title = "Awen - Pong",
                          .width = init_width,
                          .height = init_height,
                          .flags = {awn::graphics::ecs::Window::ConfigFlag::resizable, awn::graphics::ecs::Window::ConfigFlag::high_dpi},
                      });

    return engine.run();
}
catch (...)
{
    return EXIT_FAILURE;
}
