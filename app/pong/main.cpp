#include <memory>
#include <print>

#include <chrono>

import awen.core;

auto main() -> int
try
{
    awn::core::Engine engine;

    // auto window = std::make_unique<awn::scene::Window>();

    //

    auto window = std::make_unique<awn::core::Object>();
    window->set_name("Main Window");

    auto paddle_left = std::make_unique<awn::core::Object>();
    paddle_left->set_name("Left Paddle");

    auto paddle_body = std::make_unique<awn::core::Object>();
    paddle_body->set_name("Paddle Body");
    paddle_left->add_child(std::move(paddle_body));

    window->add_child(std::move(paddle_left));

    engine.add_child(std::move(window));

    // window.run();
    return engine.run();
}
catch (...)
{
    return EXIT_FAILURE;
}