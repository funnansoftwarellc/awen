#include <SDL3/SDL_main.h>
#include <SDL3/SDL_scancode.h>

#include <awen/flecs/Flecs.hpp>

#include <glm/vec2.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numbers>
#include <random>
#include <string>
#include <variant>

import awen.sdl;

using awen::core::EnumMask;
using awen::sdl::AppState;
using awen::sdl::Circle;
using awen::sdl::Drawable;
using awen::sdl::FrameTiming;
using awen::sdl::KeyboardState;
using awen::sdl::loadFont;
using awen::sdl::LocalTransform;
using awen::sdl::MouseState;
using awen::sdl::Rectangle;
using awen::sdl::TextLabel;
using awen::sdl::Window;
using awen::sdl::WindowHandles;
using awen::sdl::ZOrder;
namespace colors = awen::sdl::colors;
namespace phases = awen::sdl::phases;

namespace
{
    constexpr auto InitWidth = 1280;
    constexpr auto InitHeight = 720;
    constexpr auto WindowPositionX = 80;
    constexpr auto WindowPositionY = 80;

    constexpr auto PaddleWidth = 14.0F;
    constexpr auto PaddleHeight = 90.0F;
    constexpr auto PaddleSpeed = 380.0F;
    constexpr auto PaddleOffset = 128.0F;

    constexpr auto BallRadius = 10.0F;
    constexpr auto BallInitSpeed = 300.0F;
    constexpr auto BallSpeedUp = 1.05F;
    constexpr auto BallMaxSpeed = 700.0F;
    constexpr auto MaxBounceAngle = 60.0F;
    constexpr auto BallLaunchAngle = 30;

    constexpr auto AiSpeedRatio = 0.85F;
    constexpr auto Half = 0.5F;
    constexpr auto Deg2Rad = std::numbers::pi_v<float> / 180.0F;

    constexpr auto DashWidth = 4.0F;
    constexpr auto DashHeight = 10.0F;
    constexpr auto DashGap = 20;
    constexpr auto MaxDashes = 220;

    constexpr auto ScoreFontSize = 52;
    constexpr auto HintFontSize = 16;
    constexpr auto ScoreOffsetFromCenter = 70.0F;
    constexpr auto ScoreY = 18.0F;
    constexpr auto HintMargin = 10.0F;
    constexpr auto HintYFromBottom = 22.0F;

    /// @brief Per-side paddle state.
    struct Paddle
    {
        float y{};
        int score{};
    };

    /// @brief Ball position + velocity.
    struct Ball
    {
        glm::vec2 position{};
        glm::vec2 velocity{};
    };

    /// @brief Singleton game state owned by the world.
    struct PongState
    {
        Paddle leftPad;
        Paddle rightPad;
        Ball ball;
        bool p2Ai{true};
        bool spaceLatch{};
    };

    /// @brief Tag components used to look up the paddle, ball and HUD entities.
    struct LeftPaddleTag
    {
    };

    struct RightPaddleTag
    {
    };

    struct BallTag
    {
    };

    struct LeftScoreTag
    {
    };

    struct RightScoreTag
    {
    };

    struct LeftHintTag
    {
    };

    struct RightHintTag
    {
    };

    struct DashTag
    {
        int index{};
    };

    auto randomValue(int min, int max) -> int
    {
        static auto rng = std::mt19937{std::random_device{}()};

        return std::uniform_int_distribution<int>{min, max}(rng);
    }

    auto clampPaddle(Paddle& paddle, float screenHeight) -> void
    {
        paddle.y = std::max(0.0F, paddle.y);
        paddle.y = std::min(screenHeight - PaddleHeight, paddle.y);
    }

    auto resetBall(Ball& ball, int direction, glm::vec2 screen) -> void
    {
        ball.position = screen * Half;
        const auto angle = static_cast<float>(randomValue(-BallLaunchAngle, BallLaunchAngle)) * Deg2Rad;
        ball.velocity = glm::vec2{static_cast<float>(direction) * BallInitSpeed * std::cos(angle), BallInitSpeed * std::sin(angle)};
    }

    auto boostedSpeed(glm::vec2 v) -> float
    {
        const auto speed = std::sqrt((v.x * v.x) + (v.y * v.y)) * BallSpeedUp;

        return std::min(speed, BallMaxSpeed);
    }

    auto updateAi(Paddle& paddle, const Ball& ball, glm::vec2 screen, float deltaTime) -> void
    {
        const auto center = paddle.y + (PaddleHeight * Half);
        const auto diff = ball.position.y - center;
        const auto move = PaddleSpeed * AiSpeedRatio * deltaTime;

        if (std::abs(diff) < move)
        {
            paddle.y += diff;
        }
        else
        {
            paddle.y += (diff > 0.0F) ? move : -move;
        }

        clampPaddle(paddle, screen.y);
    }

    auto updatePhysics(PongState& state, glm::vec2 screen, float dt) -> void
    {
        state.ball.position += state.ball.velocity * dt;

        if (state.ball.position.y - BallRadius < 0.0F)
        {
            state.ball.position.y = BallRadius;
            state.ball.velocity.y = std::abs(state.ball.velocity.y);
        }

        if (state.ball.position.y + BallRadius > screen.y)
        {
            state.ball.position.y = screen.y - BallRadius;
            state.ball.velocity.y = -std::abs(state.ball.velocity.y);
        }

        const auto leftPaddleX = PaddleOffset;
        const auto rightPaddleX = screen.x - PaddleOffset - PaddleWidth;

        if (state.ball.velocity.x < 0.0F && state.ball.position.x - BallRadius <= leftPaddleX + PaddleWidth
            && state.ball.position.x - BallRadius >= leftPaddleX && state.ball.position.y >= state.leftPad.y
            && state.ball.position.y <= state.leftPad.y + PaddleHeight)
        {
            const auto hitPos = (state.ball.position.y - (state.leftPad.y + (PaddleHeight * Half))) / (PaddleHeight * Half);
            const auto angle = hitPos * MaxBounceAngle * Deg2Rad;
            const auto speed = boostedSpeed(state.ball.velocity);
            state.ball.velocity = glm::vec2{speed * std::cos(angle), speed * std::sin(angle)};
            state.ball.position.x = leftPaddleX + PaddleWidth + BallRadius;
        }

        if (state.ball.velocity.x > 0.0F && state.ball.position.x + BallRadius >= rightPaddleX
            && state.ball.position.x + BallRadius <= rightPaddleX + PaddleWidth && state.ball.position.y >= state.rightPad.y
            && state.ball.position.y <= state.rightPad.y + PaddleHeight)
        {
            const auto hitPos = (state.ball.position.y - (state.rightPad.y + (PaddleHeight * Half))) / (PaddleHeight * Half);
            const auto angle = hitPos * MaxBounceAngle * Deg2Rad;
            const auto speed = boostedSpeed(state.ball.velocity);
            state.ball.velocity = glm::vec2{-speed * std::cos(angle), speed * std::sin(angle)};
            state.ball.position.x = rightPaddleX - BallRadius;
        }

        if (state.ball.position.x - BallRadius > screen.x)
        {
            ++state.leftPad.score;
            resetBall(state.ball, -1, screen);
        }

        if (state.ball.position.x + BallRadius < 0.0F)
        {
            ++state.rightPad.score;
            resetBall(state.ball, 1, screen);
        }
    }

    /// @brief Resolve the current renderer-output size for the main window.
    auto windowSize(const flecs::world& world) -> glm::vec2
    {
        auto size = glm::vec2{InitWidth, InitHeight};

        world.each(
            [&size](flecs::entity, const Window&, const WindowHandles& handles)
            {
                if (handles.window != nullptr)
                {
                    auto w = 0;
                    auto h = 0;
                    SDL_GetWindowSize(handles.window, &w, &h);
                    size = glm::vec2{static_cast<float>(w), static_cast<float>(h)};
                }
            });

        return size;
    }

    /// @brief Mutate the TextLabel alternative inside an entity's Drawable.
    template <typename F>
    auto withTextLabel(flecs::entity entity, F&& fn) -> void
    {
        auto& drawable = entity.get_mut<Drawable>();

        if (auto* label = std::get_if<TextLabel>(&drawable.shape))
        {
            std::forward<F>(fn)(*label);
        }
    }
}

auto main(int /*argc*/, char* /*argv*/[]) -> int
try
{
    auto world = flecs::world{};
    world.import<awen::sdl::Module>();

    world.entity("MainWindow")
        .set<Window>({
            .title = "Awen SDL Pong",
            .color = colors::Black,
            .flags = EnumMask{Window::Flags::Resizable, Window::Flags::HighPixelDensity},
            .x = WindowPositionX,
            .y = WindowPositionY,
            .width = InitWidth,
            .height = InitHeight,
        });

    const auto fontEntity = loadFont(world, "fonts/DejaVuSans.ttf", ScoreFontSize);
    const auto hintFontEntity = loadFont(world, "fonts/DejaVuSans.ttf", HintFontSize);

    world.set<PongState>(PongState{
        .leftPad = {.y = (InitHeight * Half) - (PaddleHeight * Half), .score = 0},
        .rightPad = {.y = (InitHeight * Half) - (PaddleHeight * Half), .score = 0},
        .ball = {},
        .p2Ai = true,
    });

    {
        auto& state = world.get_mut<PongState>();
        resetBall(state.ball, 1, glm::vec2{InitWidth, InitHeight});
    }

    for (auto i = 0; i < MaxDashes; ++i)
    {
        world.entity()
            .set<DashTag>({.index = i})
            .set<LocalTransform>({.position = {-DashWidth, -DashHeight}})
            .set<Drawable>({.shape = Rectangle{.size = {DashWidth, DashHeight}, .anchor = {0.0F, 0.0F}, .color = colors::DarkGray}})
            .set<ZOrder>({.value = 0});
    }

    world.entity("LeftPaddle")
        .add<LeftPaddleTag>()
        .set<LocalTransform>({.position = {PaddleOffset, 0.0F}})
        .set<Drawable>({.shape = Rectangle{.size = {PaddleWidth, PaddleHeight}, .anchor = {0.0F, 0.0F}, .color = colors::White}})
        .set<ZOrder>({.value = 10});

    world.entity("RightPaddle")
        .add<RightPaddleTag>()
        .set<LocalTransform>({.position = {0.0F, 0.0F}})
        .set<Drawable>({.shape = Rectangle{.size = {PaddleWidth, PaddleHeight}, .anchor = {0.0F, 0.0F}, .color = colors::White}})
        .set<ZOrder>({.value = 10});

    world.entity("Ball")
        .add<BallTag>()
        .set<LocalTransform>({.position = {0.0F, 0.0F}})
        .set<Drawable>({.shape = Circle{.radius = BallRadius, .color = colors::White}})
        .set<ZOrder>({.value = 10});

    if (fontEntity.is_valid())
    {
        world.entity("LeftScore")
            .add<LeftScoreTag>()
            .set<LocalTransform>({.position = {0.0F, ScoreY}})
            .set<Drawable>({.shape = TextLabel{.text = "0", .font = fontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::White}})
            .set<ZOrder>({.value = 5});

        world.entity("RightScore")
            .add<RightScoreTag>()
            .set<LocalTransform>({.position = {0.0F, ScoreY}})
            .set<Drawable>({.shape = TextLabel{.text = "0", .font = fontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::White}})
            .set<ZOrder>({.value = 5});
    }

    if (hintFontEntity.is_valid())
    {
        world.entity("LeftHint")
            .add<LeftHintTag>()
            .set<LocalTransform>({.position = {HintMargin, 0.0F}})
            .set<Drawable>({.shape = TextLabel{.text = "W / S", .font = hintFontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::DarkGray}})
            .set<ZOrder>({.value = 5});

        world.entity("RightHint")
            .add<RightHintTag>()
            .set<LocalTransform>({.position = {0.0F, 0.0F}})
            .set<Drawable>(
                {.shape = TextLabel{.text = "P2: AI  [SPACE]", .font = hintFontEntity.id(), .anchor = {0.0F, 0.0F}, .color = colors::DarkGray}})
            .set<ZOrder>({.value = 5});
    }

    // Input + AI: read keyboard/mouse, update paddle Y in PongState.
    world.system("PongInput")
        .kind<phases::OnUpdate>()
        .run(
            [](flecs::iter& it)
            {
                const auto& world = it.world();
                auto& state = world.get_mut<PongState>();
                const auto& keyboard = world.get<KeyboardState>();
                const auto& mouse = world.get<MouseState>();
                const auto screen = windowSize(world);
                const auto dt = it.delta_time();

                if (keyboard.wasPressed(SDL_SCANCODE_ESCAPE))
                {
                    world.get_mut<AppState>().running = false;
                }

                if (keyboard.wasPressed(SDL_SCANCODE_SPACE))
                {
                    state.p2Ai = !state.p2Ai;
                }

                if (keyboard.isDown(SDL_SCANCODE_W))
                {
                    state.leftPad.y -= PaddleSpeed * dt;
                }

                if (keyboard.isDown(SDL_SCANCODE_S))
                {
                    state.leftPad.y += PaddleSpeed * dt;
                }

                if (mouse.isButtonDown(SDL_BUTTON_LEFT))
                {
                    state.leftPad.y = mouse.position.y - (PaddleHeight * Half);
                }

                clampPaddle(state.leftPad, screen.y);

                if (state.p2Ai)
                {
                    updateAi(state.rightPad, state.ball, screen, dt);
                }
                else
                {
                    if (keyboard.isDown(SDL_SCANCODE_UP))
                    {
                        state.rightPad.y -= PaddleSpeed * dt;
                    }

                    if (keyboard.isDown(SDL_SCANCODE_DOWN))
                    {
                        state.rightPad.y += PaddleSpeed * dt;
                    }

                    clampPaddle(state.rightPad, screen.y);
                }
            });

    // Physics: ball motion, collisions, scoring.
    world.system("PongPhysics")
        .kind<phases::OnPhysics>()
        .run(
            [](flecs::iter& it)
            {
                const auto& world = it.world();
                auto& state = world.get_mut<PongState>();
                const auto screen = windowSize(world);

                updatePhysics(state, screen, it.delta_time());
            });

    // Sync game state into entity transforms / labels.
    world.system("PongSync")
        .kind<phases::OnPreRender>()
        .run(
            [](flecs::iter& it)
            {
                const auto& world = it.world();
                const auto& state = world.get<PongState>();
                const auto screen = windowSize(world);
                const auto halfWidth = screen.x * Half;

                world.each([&state](flecs::entity, const LeftPaddleTag&, LocalTransform& t)
                           { t.position = glm::vec2{PaddleOffset, state.leftPad.y}; });

                world.each([&state, screen](flecs::entity, const RightPaddleTag&, LocalTransform& t)
                           { t.position = glm::vec2{screen.x - PaddleOffset - PaddleWidth, state.rightPad.y}; });

                world.each([&state](flecs::entity, const BallTag&, LocalTransform& t) { t.position = state.ball.position; });

                world.each(
                    [&state, halfWidth](flecs::entity entity, const LeftScoreTag&, LocalTransform& t)
                    {
                        t.position = glm::vec2{halfWidth - ScoreOffsetFromCenter, ScoreY};

                        withTextLabel(entity,
                                      [&state](TextLabel& label)
                                      {
                                          const auto str = std::to_string(state.leftPad.score);

                                          if (label.text != str)
                                          {
                                              label.text = str;
                                          }
                                      });
                    });

                world.each(
                    [&state, halfWidth](flecs::entity entity, const RightScoreTag&, LocalTransform& t)
                    {
                        t.position = glm::vec2{halfWidth + ScoreOffsetFromCenter, ScoreY};

                        withTextLabel(entity,
                                      [&state](TextLabel& label)
                                      {
                                          const auto str = std::to_string(state.rightPad.score);

                                          if (label.text != str)
                                          {
                                              label.text = str;
                                          }

                                          // Anchor right side to the right edge of the score area.
                                          label.anchor = glm::vec2{1.0F, 0.0F};
                                      });
                    });

                world.each([screen](flecs::entity, const LeftHintTag&, LocalTransform& t)
                           { t.position = glm::vec2{HintMargin, screen.y - HintYFromBottom}; });

                world.each(
                    [&state, screen](flecs::entity entity, const RightHintTag&, LocalTransform& t)
                    {
                        t.position = glm::vec2{screen.x - HintMargin, screen.y - HintYFromBottom};

                        withTextLabel(entity,
                                      [&state](TextLabel& label)
                                      {
                                          const auto* p2Text = state.p2Ai ? "P2: AI  [SPACE]" : "UP/DOWN  [SPACE]";

                                          if (label.text != p2Text)
                                          {
                                              label.text = p2Text;
                                          }

                                          label.anchor = glm::vec2{1.0F, 0.0F};
                                      });
                    });

                world.each(
                    [halfWidth, screen](flecs::entity, const DashTag& tag, LocalTransform& t)
                    {
                        const auto y = static_cast<float>(tag.index * DashGap);

                        if (y >= screen.y)
                        {
                            t.position = glm::vec2{-DashWidth, -DashHeight};
                        }
                        else
                        {
                            t.position = glm::vec2{halfWidth - (DashWidth * Half), y};
                        }
                    });
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
