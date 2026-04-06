#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <random>
#include <string>
#include <tuple>
#include <vector>

import awen.engine;

namespace
{
    // ── Constants ─────────────────────────────────────────────────────────
    constexpr auto InitWidth = 1280;
    constexpr auto InitHeight = 720;
    constexpr auto PaddleWidth = 14.0F;
    constexpr auto PaddleHeight = 90.0F;
    constexpr auto PaddleSpeed = 380.0F;
    constexpr auto PaddleOffset = 128.0F;
    constexpr auto BallRadius = 10.0F;
    constexpr auto BallInitSpeed = 300.0F;
    constexpr auto BallSpeedUp = 1.05F;
    constexpr auto BallMaxSpeed = 700.0F;
    constexpr auto MaxBounceAngle = 60.0F;
    constexpr auto Half = 0.5F;
    constexpr auto AiSpeedRatio = 0.85F;
    constexpr auto BallLaunchAngle = 30;
    constexpr auto TargetFps = 60;
    constexpr auto DashWidth = 4.0F;
    constexpr auto DashCenterOffset = 2.0F;
    constexpr auto DashGap = 20;
    constexpr auto DashHeight = 10;
    constexpr auto ScoreXLeft = 70;
    constexpr auto ScoreY = 18;
    constexpr auto ScoreFontSize = 52;
    constexpr auto HintX = 10;
    constexpr auto HintYFromBottom = 22;
    constexpr auto HintFontSize = 16;
    constexpr auto Deg2Rad = std::numbers::pi_v<float> / 180.0F;
    constexpr auto MaxDashes = 220;
    constexpr auto ClearAlpha = 120;

    struct Paddle
    {
        float x{};
        float y{};
        int score{};
    };

    struct Ball
    {
        float x{};
        float y{};
        float vx{};
        float vy{};
    };

    struct ScreenSize
    {
        float w{};
        float h{};
    };

    struct GameState
    {
        Paddle leftPad;
        Paddle rightPad;
        Ball ball;
        bool p2Ai{true};
    };

    auto randomValue(int min, int max) -> int
    {
        static auto rng = std::mt19937{std::random_device{}()};
        return std::uniform_int_distribution<int>{min, max}(rng);
    }

    auto clampPaddle(Paddle& p, float screenHeight) -> void
    {
        p.y = std::max(0.0F, p.y);
        p.y = std::min(screenHeight - PaddleHeight, p.y);
    }

    auto resetBall(Ball& ball, int direction, ScreenSize screen) -> void
    {
        ball.x = screen.w * Half;
        ball.y = screen.h * Half;
        const auto angle = static_cast<float>(randomValue(-BallLaunchAngle, BallLaunchAngle)) * Deg2Rad;
        ball.vx = static_cast<float>(direction) * BallInitSpeed * std::cos(angle);
        ball.vy = BallInitSpeed * std::sin(angle);
    }

    auto boostedSpeed(float vx, float vy) -> float
    {
        const auto s = std::sqrt((vx * vx) + (vy * vy)) * BallSpeedUp;
        return std::min(s, BallMaxSpeed);
    }

    auto updateAi(Paddle& paddle, const Ball& ball, ScreenSize screen, float deltaTime) -> void
    {
        const auto center = paddle.y + (PaddleHeight * Half);
        const auto diff = ball.y - center;
        const auto move = PaddleSpeed * AiSpeedRatio * deltaTime;

        if (std::abs(diff) < move)
        {
            paddle.y += diff;
        }
        else
        {
            paddle.y += (diff > 0.0F) ? move : -move;
        }

        clampPaddle(paddle, screen.h);
    }

    auto updatePhysics(GameState& state, ScreenSize screen, float dt) -> void
    {
        state.ball.x += state.ball.vx * dt;
        state.ball.y += state.ball.vy * dt;

        // Wall collision top.
        if (state.ball.y - BallRadius < 0.0F)
        {
            state.ball.y = BallRadius;
            state.ball.vy = std::abs(state.ball.vy);
        }

        // Wall collision bottom.
        if (state.ball.y + BallRadius > screen.h)
        {
            state.ball.y = screen.h - BallRadius;
            state.ball.vy = -std::abs(state.ball.vy);
        }

        // Left paddle collision.
        if (state.ball.vx < 0.0F && state.ball.x - BallRadius <= state.leftPad.x + PaddleWidth && state.ball.x - BallRadius >= state.leftPad.x
            && state.ball.y >= state.leftPad.y && state.ball.y <= state.leftPad.y + PaddleHeight)
        {
            const auto hitPos = (state.ball.y - (state.leftPad.y + (PaddleHeight * Half))) / (PaddleHeight * Half);
            const auto angle = hitPos * MaxBounceAngle * Deg2Rad;
            const auto speed = boostedSpeed(state.ball.vx, state.ball.vy);
            state.ball.vx = speed * std::cos(angle);
            state.ball.vy = speed * std::sin(angle);
            state.ball.x = state.leftPad.x + PaddleWidth + BallRadius;
        }

        // Right paddle collision.
        if (state.ball.vx > 0.0F && state.ball.x + BallRadius >= state.rightPad.x && state.ball.x + BallRadius <= state.rightPad.x + PaddleWidth
            && state.ball.y >= state.rightPad.y && state.ball.y <= state.rightPad.y + PaddleHeight)
        {
            const auto hitPos = (state.ball.y - (state.rightPad.y + (PaddleHeight * Half))) / (PaddleHeight * Half);
            const auto angle = hitPos * MaxBounceAngle * Deg2Rad;
            const auto speed = boostedSpeed(state.ball.vx, state.ball.vy);
            state.ball.vx = -speed * std::cos(angle);
            state.ball.vy = speed * std::sin(angle);
            state.ball.x = state.rightPad.x - BallRadius;
        }

        // Scoring.
        if (state.ball.x - BallRadius > screen.w)
        {
            ++state.leftPad.score;
            resetBall(state.ball, -1, screen);
        }

        if (state.ball.x + BallRadius < 0.0F)
        {
            ++state.rightPad.score;
            resetBall(state.ball, 1, screen);
        }
    }
}

auto main() -> int
{
    try
    {
        using namespace awen::graphics;
        using namespace awen::scene;

        auto engine = awen::Engine{"Awen - Pong", InitWidth, InitHeight, {ConfigFlag::resizable, ConfigFlag::high_dpi}};
        awen::Engine::setTargetFps(TargetFps);
        engine.setClearColor(Color{.r = 0, .g = 0, .b = 0, .a = ClearAlpha});

        const auto sw0 = static_cast<float>(InitWidth);
        const auto sh0 = static_cast<float>(InitHeight);

        auto state = GameState{
            .leftPad = {.x = PaddleOffset, .y = (sh0 * Half) - (PaddleHeight * Half), .score = 0},
            .rightPad = {.x = sw0 - PaddleOffset - PaddleWidth, .y = (sh0 * Half) - (PaddleHeight * Half), .score = 0},
            .ball = {},
            .p2Ai = true,
        };

        resetBall(state.ball, 1, ScreenSize{.w = sw0, .h = sh0});

        auto scene = Scene{};
        auto root = scene.root();

        // Dashed center line (z = 0).
        auto dashes = std::vector<NodeHandle<RectNode>>{};
        dashes.reserve(MaxDashes);

        for (auto i = 0; i < MaxDashes; ++i)
        {
            dashes.push_back(root.addChild<RectNode>().setSize(DashWidth, static_cast<float>(DashHeight)).setColor(colors::DarkGray));
        }

        // Paddles (z = 1).
        auto leftPaddle = root.addChild<RectNode>(1);
        std::ignore = leftPaddle.setSize(PaddleWidth, PaddleHeight).setColor(colors::White);

        auto rightPaddle = root.addChild<RectNode>(1);
        std::ignore = rightPaddle.setSize(PaddleWidth, PaddleHeight).setColor(colors::White);

        // Ball (z = 1).
        auto ballNode = root.addChild<CircleNode>(1);
        std::ignore = ballNode.setRadius(BallRadius).setColor(colors::White);

        // Scores (z = 2).
        auto leftScore = root.addChild<TextNode>(2);
        std::ignore = leftScore.setFontSize(ScoreFontSize).setColor(colors::White);

        auto rightScore = root.addChild<TextNode>(2);
        std::ignore = rightScore.setFontSize(ScoreFontSize).setColor(colors::White);

        // Hints (z = 2).
        auto leftHint = root.addChild<TextNode>(2);
        std::ignore = leftHint.setText("W / S").setFontSize(HintFontSize).setColor(colors::DarkGray);

        auto rightHint = root.addChild<TextNode>(2);
        std::ignore = rightHint.setFontSize(HintFontSize).setColor(colors::DarkGray);

        engine.onEvent(
            [&state](const EventKeyboard& ev)
            {
                if (ev.key == EventKeyboard::Key::space && ev.type == EventKeyboard::Type::pressed)
                {
                    state.p2Ai = !state.p2Ai;
                }
            });

        engine.run(
            scene,
            [&](float dt)
            {
                const auto sw = static_cast<float>(Window::getScreenWidth());
                const auto sh = static_cast<float>(Window::getScreenHeight());
                const auto screen = ScreenSize{.w = sw, .h = sh};

                state.rightPad.x = sw - PaddleOffset - PaddleWidth;

                // Player 1 input.
                if (Window::isKeyDown(EventKeyboard::Key::w))
                {
                    state.leftPad.y -= PaddleSpeed * dt;
                }

                if (Window::isKeyDown(EventKeyboard::Key::s))
                {
                    state.leftPad.y += PaddleSpeed * dt;
                }

                clampPaddle(state.leftPad, sh);

                // Player 2 input.
                if (state.p2Ai)
                {
                    updateAi(state.rightPad, state.ball, screen, dt);
                }
                else
                {
                    if (Window::isKeyDown(EventKeyboard::Key::up))
                    {
                        state.rightPad.y -= PaddleSpeed * dt;
                    }

                    if (Window::isKeyDown(EventKeyboard::Key::down))
                    {
                        state.rightPad.y += PaddleSpeed * dt;
                    }

                    clampPaddle(state.rightPad, sh);
                }

                updatePhysics(state, screen, dt);

                // Update scene nodes.
                const auto halfWidth = sw * Half;
                const auto screenWidthInt = static_cast<int>(sw);
                const auto screenHeightInt = static_cast<int>(sh);

                // Dashes.
                auto dashIndex = 0;

                for (auto y = 0; y < screenHeightInt && dashIndex < MaxDashes; y += DashGap, ++dashIndex)
                {
                    std::ignore = dashes[static_cast<std::size_t>(dashIndex)].setTransform(
                        Transform{.x = halfWidth - DashCenterOffset, .y = static_cast<float>(y)});
                }

                for (; dashIndex < MaxDashes; ++dashIndex)
                {
                    std::ignore =
                        dashes[static_cast<std::size_t>(dashIndex)].setTransform(Transform{.x = -DashWidth, .y = -static_cast<float>(DashHeight)});
                }

                // Paddles.
                std::ignore = leftPaddle.setTransform(Transform{.x = state.leftPad.x, .y = state.leftPad.y});
                std::ignore = rightPaddle.setTransform(Transform{.x = state.rightPad.x, .y = state.rightPad.y});

                // Ball.
                std::ignore = ballNode.setTransform(Transform{.x = state.ball.x, .y = state.ball.y});

                // Scores.
                const auto leftScoreStr = std::to_string(state.leftPad.score);
                const auto rightScoreStr = std::to_string(state.rightPad.score);

                std::ignore =
                    leftScore.setText(leftScoreStr)
                        .setTransform(Transform{.x = static_cast<float>(static_cast<int>(halfWidth) - ScoreXLeft), .y = static_cast<float>(ScoreY)});

                std::ignore = rightScore.setText(rightScoreStr)
                                  .setTransform(Transform{
                                      .x = static_cast<float>(static_cast<int>(halfWidth) + ScoreXLeft
                                                              - Renderer::measureText(rightScoreStr.c_str(), ScoreFontSize)),
                                      .y = static_cast<float>(ScoreY),
                                  });

                // Hints.
                std::ignore =
                    leftHint.setTransform(Transform{.x = static_cast<float>(HintX), .y = static_cast<float>(screenHeightInt - HintYFromBottom)});

                const auto* p2Text = state.p2Ai ? "P2: AI  [SPACE]" : "UP/DOWN  [SPACE]";

                std::ignore = rightHint.setText(p2Text).setTransform(Transform{
                    .x = static_cast<float>(screenWidthInt - Renderer::measureText(p2Text, HintFontSize) - HintX),
                    .y = static_cast<float>(screenHeightInt - HintYFromBottom),
                });
            });

        return EXIT_SUCCESS;
    }
    catch (...)
    {
        return EXIT_FAILURE;
    }
}
