#include <SDL3/SDL.h>
#include <awen/flecs/Flecs.hpp>

import awen.sdl.ecs;

using awen::core::EnumMask;
using awen::sdl::ecs::ColorRectangle;
using awen::sdl::ecs::Drawable;
using awen::sdl::ecs::Window;

auto main() -> int
try
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return EXIT_FAILURE;
    }

    auto world = flecs::world{};
    world.import<awen::sdl::ecs::Module>();

    auto ew = world.entity();

    ew.set<Window>({
        .title = "Caerwyn",
        .color = awen::sdl::ecs::colors::DarkGray,
        .flags = EnumMask{Window::Flags::Resizable, Window::Flags::HighPixelDensity},
        .x = 80,
        .y = 80,
        .width = 720,
        .height = 1280,
    });

    auto er = world.entity();
    er.set<Drawable>(ColorRectangle{
        .color = awen::sdl::ecs::colors::Red,
        .x = 100,
        .y = 100,
        .width = 200,
        .height = 150,
    });

    while (ew.get<Window>().running)
    {
        world.progress();
    }

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}