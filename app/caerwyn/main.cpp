#include <awen/flecs/Flecs.hpp>

import awen.sdl;

using awen::core::EnumMask;
using awen::sdl::AppState;
using awen::sdl::CircleFill;
using awen::sdl::CircleOutline;
using awen::sdl::Line;
using awen::sdl::LocalTransform;
using awen::sdl::Polygon;
using awen::sdl::RectangleFill;
using awen::sdl::RectangleOutline;
using awen::sdl::Window;
using awen::sdl::ZOrder;
namespace colors = awen::sdl::colors;
namespace phases = awen::sdl::phases;

auto main() -> int
try
{
    auto world = flecs::world{};
    world.import<awen::sdl::Module>();

    world.entity("MainWindow")
        .set<Window>({
            .title = "Caerwyn",
            .color = colors::DarkGray,
            .flags = EnumMask{Window::Flags::Resizable, Window::Flags::HighPixelDensity},
            .x = 80,
            .y = 80,
            .width = 1280,
            .height = 720,
        });

    world.entity().set<LocalTransform>({.position = {200.0F, 150.0F}}).set<RectangleFill>({.size = {200.0F, 120.0F}, .color = colors::Red});

    world.entity()
        .set<LocalTransform>({.position = {500.0F, 150.0F}})
        .set<RectangleOutline>({.size = {200.0F, 120.0F}, .color = colors::Yellow, .thickness = 2.0F});

    world.entity().set<LocalTransform>({.position = {800.0F, 150.0F}}).set<CircleFill>({.radius = 60.0F, .color = colors::Green});

    world.entity().set<LocalTransform>({.position = {1000.0F, 150.0F}}).set<CircleOutline>({.radius = 60.0F, .color = colors::Blue});

    world.entity()
        .set<LocalTransform>({.position = {200.0F, 350.0F}})
        .set<Line>({.from = {-80.0F, 0.0F}, .to = {80.0F, 0.0F}, .color = colors::White, .thickness = 1.0F});

    world.entity()
        .set<LocalTransform>({.position = {500.0F, 380.0F}})
        .set<Polygon>({.points = {{0.0F, -60.0F}, {60.0F, 60.0F}, {-60.0F, 60.0F}}, .color = colors::Purple, .filled = true});

    const auto parent = world.entity("HierarchyParent")
                            .set<LocalTransform>({.position = {950.0F, 420.0F}})
                            .set<RectangleFill>({.size = {120.0F, 120.0F}, .color = colors::Orange})
                            .set<ZOrder>({.value = 0});

    world.entity("HierarchyChild")
        .child_of(parent)
        .set<LocalTransform>({.position = {0.0F, 0.0F}, .scale = {0.4F, 0.4F}})
        .set<RectangleFill>({.size = {120.0F, 120.0F}, .color = colors::White})
        .set<ZOrder>({.value = 1});

    world.system<LocalTransform>("RotateHierarchyParent")
        .kind<phases::OnUpdate>()
        .each(
            [](flecs::iter& it, std::size_t i, LocalTransform& t)
            {
                const auto e = it.entity(i);

                if (e.name() == "HierarchyParent")
                {
                    t.rotation += it.delta_time();
                }
            });

    while (world.get<AppState>().running)
    {
        world.progress();
    }

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
