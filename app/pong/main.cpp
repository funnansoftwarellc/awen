#include <memory>
#include <print>

import awen.core;

auto main() -> int
try
{
    awn::core::Engine engine;

    auto window = std::make_unique<awn::Object>();
    window->set_name("Main Window");
    engine.add_child(std::move(window));

    return engine.run();
}
catch (...)
{
    return EXIT_FAILURE;
}