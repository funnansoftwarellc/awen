#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <numbers>
#include <print>
#include <random>
#include <string>
#include <vector>

#include <chrono>

import awen.core;
import awen.widget;

namespace
{
    constexpr auto InitWidth = 1280.0F;
    constexpr auto InitHeight = 720.0F;
    constexpr auto WindowPositionX = 80.0F;
    constexpr auto WindowPositionY = 80.0F;
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

    auto clampPaddle(Paddle& paddle, float screenHeight) -> void
    {
        paddle.y = std::max(0.0F, paddle.y);
        paddle.y = std::min(screenHeight - PaddleHeight, paddle.y);
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
        const auto speed = std::sqrt((vx * vx) + (vy * vy)) * BallSpeedUp;
        return std::min(speed, BallMaxSpeed);
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

        if (state.ball.y - BallRadius < 0.0F)
        {
            state.ball.y = BallRadius;
            state.ball.vy = std::abs(state.ball.vy);
        }

        if (state.ball.y + BallRadius > screen.h)
        {
            state.ball.y = screen.h - BallRadius;
            state.ball.vy = -std::abs(state.ball.vy);
        }

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

    auto handleP2Input(Paddle& paddle, const Ball& ball, ScreenSize screen, bool isAi, float deltaTime) -> void
    {
        if (isAi)
        {
            updateAi(paddle, ball, screen, deltaTime);
        }
        else
        {
            if (awen::widget::Window::isKeyDown(awen::widget::EventKeyboard::Key::up))
            {
                paddle.y -= PaddleSpeed * deltaTime;
            }

            if (awen::widget::Window::isKeyDown(awen::widget::EventKeyboard::Key::down))
            {
                paddle.y += PaddleSpeed * deltaTime;
            }

            clampPaddle(paddle, screen.h);
        }
    }
}

auto main() -> int // NOLINT(bugprone-exception-escape)
try
{
    using awen::widget::EventKeyboard;
    using awen::widget::NodeCircle;
    using awen::widget::NodeRectangle;
    using awen::widget::NodeText;
    using awen::widget::NodeTransform;
    using awen::widget::Window;

    auto engine = awen::core::Engine{};

    auto window = std::make_unique<Window>();
    auto* windowNode = window.get();
    window->setTitle("Awen SDL Pong");
    window->setSize({InitWidth, InitHeight});
    window->setPosition({WindowPositionX, WindowPositionY});
    window->setClearColor(awen::sdl::Color{.r = 0, .g = 0, .b = 0, .a = ClearAlpha});

    const auto sw0 = InitWidth;
    const auto sh0 = InitHeight;

    auto state = GameState{
        .leftPad = {.x = PaddleOffset, .y = (sh0 * Half) - (PaddleHeight * Half), .score = 0},
        .rightPad = {.x = sw0 - PaddleOffset - PaddleWidth, .y = (sh0 * Half) - (PaddleHeight * Half), .score = 0},
        .ball = {},
        .p2Ai = true,
    };

    resetBall(state.ball, 1, ScreenSize{.w = sw0, .h = sh0});

    auto dashes = std::vector<NodeTransform*>{};
    dashes.reserve(MaxDashes);

    for (auto i = 0; i < MaxDashes; ++i)
    {
        auto dashTransform = std::make_unique<NodeTransform>();
        auto dash = std::make_unique<NodeRectangle>();
        dash->setSize({DashWidth, static_cast<float>(DashHeight)});
        dash->setColor(awen::widget::colors::DarkGray);
        dashes.push_back(dashTransform.get());
        dashTransform->addChild(std::move(dash));
        window->addChild(std::move(dashTransform));
    }

    auto leftPaddleTransform = std::make_unique<NodeTransform>();
    auto leftPaddle = std::make_unique<NodeRectangle>();
    leftPaddle->setSize({PaddleWidth, PaddleHeight});
    leftPaddle->setColor(awen::widget::colors::White);
    auto* leftPaddleNode = leftPaddleTransform.get();
    leftPaddleTransform->addChild(std::move(leftPaddle));
    window->addChild(std::move(leftPaddleTransform));

    auto rightPaddleTransform = std::make_unique<NodeTransform>();
    auto rightPaddle = std::make_unique<NodeRectangle>();
    rightPaddle->setSize({PaddleWidth, PaddleHeight});
    rightPaddle->setColor(awen::widget::colors::White);
    auto* rightPaddleNode = rightPaddleTransform.get();
    rightPaddleTransform->addChild(std::move(rightPaddle));
    window->addChild(std::move(rightPaddleTransform));

    auto ballTransform = std::make_unique<NodeTransform>();
    auto ball = std::make_unique<NodeCircle>();
    ball->setRadius(BallRadius);
    ball->setColor(awen::widget::colors::White);
    auto* ballNode = ballTransform.get();
    ballTransform->addChild(std::move(ball));
    window->addChild(std::move(ballTransform));

    auto leftScoreTransform = std::make_unique<NodeTransform>();
    auto leftScore = std::make_unique<NodeText>();
    leftScore->setFontSize(ScoreFontSize);
    leftScore->setColor(awen::widget::colors::White);
    auto* leftScoreTransformNode = leftScoreTransform.get();
    auto* leftScoreNode = leftScore.get();
    leftScoreTransform->addChild(std::move(leftScore));
    window->addChild(std::move(leftScoreTransform));

    auto rightScoreTransform = std::make_unique<NodeTransform>();
    auto rightScore = std::make_unique<NodeText>();
    rightScore->setFontSize(ScoreFontSize);
    rightScore->setColor(awen::widget::colors::White);
    auto* rightScoreTransformNode = rightScoreTransform.get();
    auto* rightScoreNode = rightScore.get();
    rightScoreTransform->addChild(std::move(rightScore));
    window->addChild(std::move(rightScoreTransform));

    auto leftHintTransform = std::make_unique<NodeTransform>();
    auto leftHint = std::make_unique<NodeText>();
    leftHint->setText("W / S");
    leftHint->setFontSize(HintFontSize);
    leftHint->setColor(awen::widget::colors::DarkGray);
    auto* leftHintTransformNode = leftHintTransform.get();
    leftHintTransform->addChild(std::move(leftHint));
    window->addChild(std::move(leftHintTransform));

    auto rightHintTransform = std::make_unique<NodeTransform>();
    auto rightHint = std::make_unique<NodeText>();
    rightHint->setFontSize(HintFontSize);
    rightHint->setColor(awen::widget::colors::DarkGray);
    auto* rightHintTransformNode = rightHintTransform.get();
    auto* rightHintNode = rightHint.get();
    rightHintTransform->addChild(std::move(rightHint));
    window->addChild(std::move(rightHintTransform));

    engine.onUpdate().connect(
        [&](std::chrono::duration<float> dt)
        {
            const auto deltaTime = dt.count();
            const auto sw = static_cast<float>(Window::getScreenWidth());
            const auto sh = static_cast<float>(Window::getScreenHeight());
            const auto screen = ScreenSize{.w = sw, .h = sh};

            state.rightPad.x = sw - PaddleOffset - PaddleWidth;

            if (Window::isKeyPressed(EventKeyboard::Key::space))
            {
                state.p2Ai = !state.p2Ai;
            }

            if (Window::isKeyDown(EventKeyboard::Key::w))
            {
                state.leftPad.y -= PaddleSpeed * deltaTime;
            }

            if (Window::isKeyDown(EventKeyboard::Key::s))
            {
                state.leftPad.y += PaddleSpeed * deltaTime;
            }

            clampPaddle(state.leftPad, sh);

            handleP2Input(state.rightPad, state.ball, screen, state.p2Ai, deltaTime);

            updatePhysics(state, screen, deltaTime);

            const auto halfWidth = sw * Half;
            const auto screenWidthInt = static_cast<int>(sw);
            const auto screenHeightInt = static_cast<int>(sh);

            auto dashIndex = 0;

            for (auto y = 0; y < screenHeightInt && dashIndex < MaxDashes; y += DashGap, ++dashIndex)
            {
                dashes[static_cast<std::size_t>(dashIndex)]->setPosition( // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
                    {halfWidth - DashCenterOffset, static_cast<float>(y)});
            }

            for (; dashIndex < MaxDashes; ++dashIndex)
            {
                dashes[static_cast<std::size_t>(dashIndex)]->setPosition( // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
                    {-DashWidth, -static_cast<float>(DashHeight)});
            }

            leftPaddleNode->setPosition({state.leftPad.x, state.leftPad.y});
            rightPaddleNode->setPosition({state.rightPad.x, state.rightPad.y});
            ballNode->setPosition({state.ball.x, state.ball.y});

            const auto leftScoreStr = std::to_string(state.leftPad.score);
            const auto rightScoreStr = std::to_string(state.rightPad.score);

            leftScoreNode->setText(leftScoreStr);
            leftScoreTransformNode->setPosition({static_cast<float>(static_cast<int>(halfWidth) - ScoreXLeft), static_cast<float>(ScoreY)});

            rightScoreNode->setText(rightScoreStr);
            rightScoreTransformNode->setPosition({
                static_cast<float>(static_cast<int>(halfWidth) + ScoreXLeft - Window::measureText(rightScoreStr.c_str(), ScoreFontSize)),
                static_cast<float>(ScoreY),
            });

            leftHintTransformNode->setPosition({static_cast<float>(HintX), static_cast<float>(screenHeightInt - HintYFromBottom)});

            const auto* p2Text = state.p2Ai ? "P2: AI  [SPACE]" : "UP/DOWN  [SPACE]";
            rightHintNode->setText(p2Text);
            rightHintTransformNode->setPosition({
                static_cast<float>(screenWidthInt - Window::measureText(p2Text, HintFontSize) - HintX),
                static_cast<float>(screenHeightInt - HintYFromBottom),
            });
        });

    engine.addChild(std::move(window));
    const auto result = engine.run();

    if (const auto& error = windowNode->getLastError(); error.has_value())
    {
        std::println(stderr, "awen-widget-pong: {}", *error);
        return EXIT_FAILURE;
    }

    return result;
}
catch (const std::exception& exception)
{
    std::println(stderr, "awen-widget-pong: {}", exception.what());
    return EXIT_FAILURE;
}
catch (...)
{
    std::println(stderr, "awen-widget-pong: Unhandled non-standard exception.");
    return EXIT_FAILURE;
}