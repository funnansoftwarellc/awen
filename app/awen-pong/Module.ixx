module;

#include <SDL3/SDL.h>
#include <awen/flecs/Flecs.hpp>

#include <glm/vec2.hpp>

#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <variant>

export module awen.pong;

export import awen.pong.actions;
export import awen.pong.components;
export import awen.pong.constants;

import awen.sdl;

namespace pong::detail
{
    using awen::sdl::AppState;
    using awen::sdl::Drawable;
    using awen::sdl::KeyboardState;
    using awen::sdl::LocalTransform;
    using awen::sdl::MouseState;
    using awen::sdl::Text;
    using awen::sdl::Window;
    using awen::sdl::WindowHandles;

    /// @brief Resolve the renderer-output size for the main window.
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

    /// @brief Mutate the Text alternative inside an entity's Drawable.
    template <typename F>
    auto withText(flecs::entity entity, const F& fn) -> void
    {
        auto& drawable = entity.get_mut<Drawable>();

        if (auto* label = std::get_if<Text>(&drawable.shape))
        {
            fn(*label);
        }
    }

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

    /// @brief Translate KeyboardState/MouseState into ActionMap state.
    auto syncActions(const KeyboardState& keyboard, const MouseState& mouse, ActionMap& map) -> void
    {
        for (auto& state : map.states)
        {
            state = ActionState{};
        }

        for (const auto& binding : map.buttons)
        {
            auto& state = map.states[static_cast<std::size_t>(binding.action)];

            if (keyboard.isDown(binding.key))
            {
                state.down = true;
                state.value = 1.0F;
            }

            if (keyboard.wasPressed(binding.key))
            {
                state.pressed = true;
            }

            if (keyboard.wasReleased(binding.key))
            {
                state.released = true;
            }
        }

        for (const auto& binding : map.axes)
        {
            auto& state = map.states[static_cast<std::size_t>(binding.action)];
            auto value = 0.0F;

            if (keyboard.isDown(binding.positive))
            {
                value += 1.0F;
            }

            if (keyboard.isDown(binding.negative))
            {
                value -= 1.0F;
            }

            state.value = value;
            state.down = value != 0.0F;
            state.pressed = keyboard.wasPressed(binding.positive) || keyboard.wasPressed(binding.negative);
            state.released = keyboard.wasReleased(binding.positive) || keyboard.wasReleased(binding.negative);
        }

        for (const auto& binding : map.mouseButtons)
        {
            auto& state = map.states[static_cast<std::size_t>(binding.action)];

            if (mouse.isButtonDown(binding.button))
            {
                state.down = true;
                state.value = 1.0F;
            }

            if (mouse.wasButtonPressed(binding.button))
            {
                state.pressed = true;
            }

            if (mouse.wasButtonReleased(binding.button))
            {
                state.released = true;
            }
        }
    }
}

export namespace pong
{
    /// @brief Pong game module. Import after `awen::sdl::Module`.
    /// @note Registers the ActionMap/PongState singletons and four systems
    ///       (ActionSync, PongInput, PongPhysics, PongSync) on the supplied
    ///       world. Default key bindings are seeded by the constructor.
    class Module
    {
    public:
        explicit Module(flecs::world& world)
        {
            world.module<Module>();

            registerSingletons(world);
            registerDefaultBindings(world);
            buildQueries(world);

            registerActionSyncSystem(world);
            registerInputSystem(world);
            registerPhysicsSystem(world);
            registerSyncSystem(world);
        }

    private:
        flecs::query<awen::sdl::LocalTransform> leftPaddleQuery_{};
        flecs::query<awen::sdl::LocalTransform> rightPaddleQuery_{};
        flecs::query<awen::sdl::LocalTransform> ballQuery_{};
        flecs::query<awen::sdl::LocalTransform> leftScoreQuery_{};
        flecs::query<awen::sdl::LocalTransform> rightScoreQuery_{};
        flecs::query<awen::sdl::LocalTransform> leftHintQuery_{};
        flecs::query<awen::sdl::LocalTransform> rightHintQuery_{};
        flecs::query<const DashTag, awen::sdl::LocalTransform> dashQuery_{};

        static auto registerSingletons(flecs::world& world) -> void
        {
            world.set<PongState>(PongState{
                .leftPad = {.y = (InitHeight * Half) - (PaddleHeight * Half), .score = 0},
                .rightPad = {.y = (InitHeight * Half) - (PaddleHeight * Half), .score = 0},
                .ball = {},
                .p2Ai = true,
            });

            auto& state = world.get_mut<PongState>();
            detail::resetBall(state.ball, 1, glm::vec2{InitWidth, InitHeight});

            world.set<ActionMap>(ActionMap{});
        }

        static auto registerDefaultBindings(flecs::world& world) -> void
        {
            auto& map = world.get_mut<ActionMap>();

            map.buttons = {
                ButtonBinding{.action = Action::Quit, .key = SDL_SCANCODE_ESCAPE},
                ButtonBinding{.action = Action::ToggleAi, .key = SDL_SCANCODE_SPACE},
            };

            map.axes = {
                AxisBinding{.action = Action::LeftPaddleAxis, .positive = SDL_SCANCODE_S, .negative = SDL_SCANCODE_W},
                AxisBinding{.action = Action::RightPaddleAxis, .positive = SDL_SCANCODE_DOWN, .negative = SDL_SCANCODE_UP},
            };

            map.mouseButtons = {
                MouseButtonBinding{.action = Action::MouseDragLeftPaddle, .button = SDL_BUTTON_LEFT},
            };
        }

        auto buildQueries(flecs::world& world) -> void
        {
            using awen::sdl::LocalTransform;

            leftPaddleQuery_ = world.query_builder<LocalTransform>().with<LeftPaddleTag>().build();
            rightPaddleQuery_ = world.query_builder<LocalTransform>().with<RightPaddleTag>().build();
            ballQuery_ = world.query_builder<LocalTransform>().with<BallTag>().build();
            leftScoreQuery_ = world.query_builder<LocalTransform>().with<LeftScoreTag>().build();
            rightScoreQuery_ = world.query_builder<LocalTransform>().with<RightScoreTag>().build();
            leftHintQuery_ = world.query_builder<LocalTransform>().with<LeftHintTag>().build();
            rightHintQuery_ = world.query_builder<LocalTransform>().with<RightHintTag>().build();
            dashQuery_ = world.query_builder<const DashTag, LocalTransform>().build();
        }

        static auto registerActionSyncSystem(flecs::world& world) -> void
        {
            world.system("pong::ActionSync")
                .kind<awen::sdl::phases::OnUpdate>()
                .run(
                    [](flecs::iter& it)
                    {
                        const auto& world = it.world();
                        const auto& keyboard = world.get<awen::sdl::KeyboardState>();
                        const auto& mouse = world.get<awen::sdl::MouseState>();
                        auto& map = world.get_mut<ActionMap>();

                        detail::syncActions(keyboard, mouse, map);
                    });
        }

        static auto registerInputSystem(flecs::world& world) -> void
        {
            world.system("pong::Input")
                .kind<awen::sdl::phases::OnUpdate>()
                .run(
                    [](flecs::iter& it)
                    {
                        const auto& world = it.world();
                        auto& state = world.get_mut<PongState>();
                        const auto& actions = world.get<ActionMap>();
                        const auto& mouse = world.get<awen::sdl::MouseState>();
                        const auto screen = detail::windowSize(world);
                        const auto dt = it.delta_time();

                        if (actions.wasPressed(Action::Quit))
                        {
                            world.get_mut<awen::sdl::AppState>().running = false;
                        }

                        if (actions.wasPressed(Action::ToggleAi))
                        {
                            state.p2Ai = !state.p2Ai;
                        }

                        state.leftPad.y += actions.axis(Action::LeftPaddleAxis) * PaddleSpeed * dt;

                        if (actions.isDown(Action::MouseDragLeftPaddle))
                        {
                            state.leftPad.y = mouse.position.y - (PaddleHeight * Half);
                        }

                        detail::clampPaddle(state.leftPad, screen.y);

                        if (state.p2Ai)
                        {
                            detail::updateAi(state.rightPad, state.ball, screen, dt);
                        }
                        else
                        {
                            state.rightPad.y += actions.axis(Action::RightPaddleAxis) * PaddleSpeed * dt;
                            detail::clampPaddle(state.rightPad, screen.y);
                        }
                    });
        }

        static auto registerPhysicsSystem(flecs::world& world) -> void
        {
            world.system("pong::Physics")
                .kind<awen::sdl::phases::OnPhysics>()
                .run(
                    [](flecs::iter& it)
                    {
                        const auto& world = it.world();
                        auto& state = world.get_mut<PongState>();
                        const auto screen = detail::windowSize(world);

                        detail::updatePhysics(state, screen, it.delta_time());
                    });
        }

        auto registerSyncSystem(flecs::world& world) -> void
        {
            world.system("pong::Sync")
                .kind<awen::sdl::phases::OnPreRender>()
                .run(
                    [this](flecs::iter& it)
                    {
                        using awen::sdl::LocalTransform;
                        using awen::sdl::Text;

                        const auto& world = it.world();
                        const auto& state = world.get<PongState>();
                        const auto screen = detail::windowSize(world);
                        const auto halfWidth = screen.x * Half;

                        leftPaddleQuery_.each([&state](flecs::entity, LocalTransform& t) { t.position = glm::vec2{PaddleOffset, state.leftPad.y}; });

                        rightPaddleQuery_.each([&state, screen](flecs::entity, LocalTransform& t)
                                               { t.position = glm::vec2{screen.x - PaddleOffset - PaddleWidth, state.rightPad.y}; });

                        ballQuery_.each([&state](flecs::entity, LocalTransform& t) { t.position = state.ball.position; });

                        leftScoreQuery_.each(
                            [&state, halfWidth](flecs::entity entity, LocalTransform& t)
                            {
                                t.position = glm::vec2{halfWidth - ScoreOffsetFromCenter, ScoreY};

                                detail::withText(entity,
                                                 [&state](Text& label)
                                                 {
                                                     const auto str = std::to_string(state.leftPad.score);

                                                     if (label.text != str)
                                                     {
                                                         label.text = str;
                                                     }
                                                 });
                            });

                        rightScoreQuery_.each(
                            [&state, halfWidth](flecs::entity entity, LocalTransform& t)
                            {
                                t.position = glm::vec2{halfWidth + ScoreOffsetFromCenter, ScoreY};

                                detail::withText(entity,
                                                 [&state](Text& label)
                                                 {
                                                     const auto str = std::to_string(state.rightPad.score);

                                                     if (label.text != str)
                                                     {
                                                         label.text = str;
                                                     }

                                                     label.anchor = glm::vec2{1.0F, 0.0F};
                                                 });
                            });

                        leftHintQuery_.each([screen](flecs::entity, LocalTransform& t)
                                            { t.position = glm::vec2{HintMargin, screen.y - HintYFromBottom}; });

                        rightHintQuery_.each(
                            [&state, screen](flecs::entity entity, LocalTransform& t)
                            {
                                t.position = glm::vec2{screen.x - HintMargin, screen.y - HintYFromBottom};

                                detail::withText(entity,
                                                 [&state](Text& label)
                                                 {
                                                     const auto* p2Text = state.p2Ai ? "P2: AI  [SPACE]" : "UP/DOWN  [SPACE]";

                                                     if (label.text != p2Text)
                                                     {
                                                         label.text = p2Text;
                                                     }

                                                     label.anchor = glm::vec2{1.0F, 0.0F};
                                                 });
                            });

                        dashQuery_.each(
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
        }
    };
}
