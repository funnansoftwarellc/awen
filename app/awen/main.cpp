#include <raylib.h>
#include <cstdlib>
#include <tuple>
#include <typeinfo>

import awen.core;
import awen.raylib;

using awen::raylib::Window;

auto main() -> int
{
    constexpr auto width{1280};
    constexpr auto height{720};

    awen::core::Engine engine;

    auto* window = engine.addChild<Window>(Window::Traits{
        .title = "Awen",
        .width = width,
        .height = height,
    });

    std::ignore = window->addChild<awen::raylib::Text>();

    return engine.run();
}