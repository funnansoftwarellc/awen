#include <SDL3/SDL_main.h>

#include <awen/flecs/Flecs.hpp>

#include <cstdlib>

import awen.pong;
import awen.sdl;

using awen::core::EnumMask;
using awen::sdl::AppState;
using awen::sdl::Circle;
using awen::sdl::Drawable;
using awen::sdl::loadFont;
using awen::sdl::LocalTransform;
using awen::sdl::Rectangle;
using awen::sdl::Text;
using awen::sdl::Window;
using awen::sdl::ZOrder;
namespace colors = awen::sdl::colors;

auto main(int /*argc*/, char* /*argv*/[]) -> int
try
{
    auto world = flecs::world{};
    world.import<awen::sdl::Module>();
    world.import<pong::Module>();

    world.entity("MainWindow")
        .set<Window>({
            .title = "Awen SDL Pong",
            .color = colors::Black,
            .flags = EnumMask{Window::Flags::Resizable, Window::Flags::HighPixelDensity},
            .x = pong::WindowPositionX,
            .y = pong::WindowPositionY,
            .width = pong::InitWidth,
            .height = pong::InitHeight,
        });

    const auto fontEntity = loadFont(world, "fonts/DejaVuSans.ttf", pong::ScoreFontSize);
    const auto hintFontEntity = loadFont(world, "fonts/DejaVuSans.ttf", pong::HintFontSize);

    const auto paddlePrefab =
        world.prefab("PaddlePrefab")
            .set<Drawable>({.shape = Rectangle{.size = {pong::PaddleWidth, pong::PaddleHeight}, .anchor = {0.0F, 0.0F}, .color = colors::White}})
            .set<ZOrder>({.value = 10});

    const auto dashPrefab =
        world.prefab("DashPrefab")
            .set<Drawable>({.shape = Rectangle{.size = {pong::DashWidth, pong::DashHeight}, .anchor = {0.0F, 0.0F}, .color = colors::DarkGray}})
            .set<ZOrder>({.value = 0});

    for (auto i = 0; i < pong::MaxDashes; ++i)
    {
        world.entity().is_a(dashPrefab).set<pong::DashTag>({.index = i}).set<LocalTransform>({.position = {-pong::DashWidth, -pong::DashHeight}});
    }

    world.entity("LeftPaddle").is_a(paddlePrefab).add<pong::LeftPaddleTag>().set<LocalTransform>({.position = {pong::PaddleOffset, 0.0F}});

    world.entity("RightPaddle").is_a(paddlePrefab).add<pong::RightPaddleTag>().set<LocalTransform>({.position = {0.0F, 0.0F}});

    world.entity("Ball")
        .add<pong::BallTag>()
        .set<LocalTransform>({.position = {0.0F, 0.0F}})
        .set<Drawable>({.shape = Circle{.radius = pong::BallRadius, .color = colors::White}})
        .set<ZOrder>({.value = 10});

    if (fontEntity.is_valid())
    {
        world.entity("LeftScore")
            .add<pong::LeftScoreTag>()
            .set<LocalTransform>({.position = {0.0F, pong::ScoreY}})
            .set<Drawable>({.shape = Text{.text = "0", .font = fontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::White}})
            .set<ZOrder>({.value = 5});

        world.entity("RightScore")
            .add<pong::RightScoreTag>()
            .set<LocalTransform>({.position = {0.0F, pong::ScoreY}})
            .set<Drawable>({.shape = Text{.text = "0", .font = fontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::White}})
            .set<ZOrder>({.value = 5});
    }

    if (hintFontEntity.is_valid())
    {
        world.entity("LeftHint")
            .add<pong::LeftHintTag>()
            .set<LocalTransform>({.position = {pong::HintMargin, 0.0F}})
            .set<Drawable>({.shape = Text{.text = "W / S", .font = hintFontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::DarkGray}})
            .set<ZOrder>({.value = 5});

        world.entity("RightHint")
            .add<pong::RightHintTag>()
            .set<LocalTransform>({.position = {0.0F, 0.0F}})
            .set<Drawable>({.shape = Text{.text = "P2: AI  [SPACE]", .font = hintFontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::DarkGray}})
            .set<ZOrder>({.value = 5});
    }

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
