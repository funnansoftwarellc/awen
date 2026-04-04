#include <print>

import awen.core;

auto main() -> int
try
{
    awn::core::Engine engine;

    // awn::widget::Engine engine;
    // awn::widget::WidgetWindow window{"Awen - Pong", 1280, 720, {awn::graphics::ConfigFlag::resizable, awn::graphics::ConfigFlag::high_dpi}};
    // engine.run(window);
    return engine.run();
}
catch (...)
{
    return EXIT_FAILURE;
}