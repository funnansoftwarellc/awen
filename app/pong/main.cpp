#include <memory>
#include <print>

#include <chrono>

import awen.core;
import awen.widget;

namespace
{
    constexpr auto WindowWidth = 1280.0F;
    constexpr auto WindowHeight = 720.0F;
    constexpr auto WindowPositionX = 80.0F;
    constexpr auto WindowPositionY = 80.0F;
}

auto main() -> int
try
{
    awen::core::Engine engine;

    auto window = std::make_unique<awen::widget::Window>();
    window->setTitle("Pong");
    window->setSize({WindowWidth, WindowHeight});
    window->setPosition({WindowPositionX, WindowPositionY});

    engine.addChild(std::move(window));

    return engine.run();
}
catch (...)
{
    return EXIT_FAILURE;
}