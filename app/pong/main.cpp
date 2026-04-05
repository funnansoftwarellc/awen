#include <memory>
#include <print>

#include <chrono>

import awen.core;
import awen.widget;

auto main() -> int
try
{
    awen::core::Engine engine;

    auto window = std::make_unique<awen::widget::Window>();
    window->set_title("Pong");
    window->set_size({1280.0F, 720.0F});
    window->set_position({80.0F, 80.0F});

    engine.add_child(std::move(window));

    return engine.run();
}
catch (...)
{
    return EXIT_FAILURE;
}