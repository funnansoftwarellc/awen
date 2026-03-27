#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <random>
#include <string>
#include <vector>

import awen.engine;

namespace
{
    // ── Constants ─────────────────────────────────────────────────────────
    constexpr auto init_width = 1280;
    constexpr auto init_height = 720;
    constexpr auto paddle_width = 14.0F;
    constexpr auto paddle_height = 90.0F;
    constexpr auto paddle_speed = 380.0F;
    constexpr auto paddle_offset = 128.0F;
    constexpr auto ball_radius = 10.0F;
    constexpr auto ball_init_speed = 300.0F;
    constexpr auto ball_speed_up = 1.05F;
    constexpr auto ball_max_speed = 700.0F;
    constexpr auto max_bounce_angle = 60.0F;
    constexpr auto half = 0.5F;
    constexpr auto ai_speed_ratio = 0.85F;
    constexpr auto ball_launch_angle = 30;
    constexpr auto target_fps = 60;
    constexpr auto dash_width = 4.0F;
    constexpr auto dash_center_offset = 2.0F;
    constexpr auto dash_gap = 20;
    constexpr auto dash_height = 10;
    constexpr auto score_x_left = 70;
    constexpr auto score_y = 18;
    constexpr auto score_font_size = 52;
    constexpr auto hint_x = 10;
    constexpr auto hint_y_from_bottom = 22;
    constexpr auto hint_font_size = 16;
    constexpr auto deg2rad = std::numbers::pi_v<float> / 180.0F;
    constexpr auto max_dashes = 220;

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
        Paddle left_pad;
        Paddle right_pad;
        Ball ball;
        bool p2_ai{true};
    };

    auto random_value(int min, int max) -> int
    {
        static auto rng = std::mt19937{std::random_device{}()};
        return std::uniform_int_distribution<int>{min, max}(rng);
    }

    auto clamp_paddle(Paddle& p, float screen_h) -> void
    {
        p.y = std::max(0.0F, p.y);
        p.y = std::min(screen_h - paddle_height, p.y);
    }

    auto reset_ball(Ball& ball, int direction, ScreenSize screen) -> void
    {
        ball.x = screen.w * half;
        ball.y = screen.h * half;
        const auto angle = static_cast<float>(random_value(-ball_launch_angle, ball_launch_angle)) * deg2rad;
        ball.vx = static_cast<float>(direction) * ball_init_speed * std::cos(angle);
        ball.vy = ball_init_speed * std::sin(angle);
    }

    auto boosted_speed(float vx, float vy) -> float
    {
        const auto s = std::sqrt((vx * vx) + (vy * vy)) * ball_speed_up;
        return std::min(s, ball_max_speed);
    }

    auto update_ai(Paddle& paddle, const Ball& ball, ScreenSize screen, float delta_time) -> void
    {
        const auto center = paddle.y + (paddle_height * half);
        const auto diff = ball.y - center;
        const auto move = paddle_speed * ai_speed_ratio * delta_time;

        if (std::abs(diff) < move)
        {
            paddle.y += diff;
        }
        else
        {
            paddle.y += (diff > 0.0F) ? move : -move;
        }

        clamp_paddle(paddle, screen.h);
    }

    auto update_physics(GameState& state, ScreenSize screen, float dt) -> void
    {
        state.ball.x += state.ball.vx * dt;
        state.ball.y += state.ball.vy * dt;

        // Wall collision top.
        if (state.ball.y - ball_radius < 0.0F)
        {
            state.ball.y = ball_radius;
            state.ball.vy = std::abs(state.ball.vy);
        }

        // Wall collision bottom.
        if (state.ball.y + ball_radius > screen.h)
        {
            state.ball.y = screen.h - ball_radius;
            state.ball.vy = -std::abs(state.ball.vy);
        }

        // Left paddle collision.
        if (state.ball.vx < 0.0F && state.ball.x - ball_radius <= state.left_pad.x + paddle_width && state.ball.x - ball_radius >= state.left_pad.x
            && state.ball.y >= state.left_pad.y && state.ball.y <= state.left_pad.y + paddle_height)
        {
            const auto hit_pos = (state.ball.y - (state.left_pad.y + (paddle_height * half))) / (paddle_height * half);
            const auto angle = hit_pos * max_bounce_angle * deg2rad;
            const auto speed = boosted_speed(state.ball.vx, state.ball.vy);
            state.ball.vx = speed * std::cos(angle);
            state.ball.vy = speed * std::sin(angle);
            state.ball.x = state.left_pad.x + paddle_width + ball_radius;
        }

        // Right paddle collision.
        if (state.ball.vx > 0.0F && state.ball.x + ball_radius >= state.right_pad.x && state.ball.x + ball_radius <= state.right_pad.x + paddle_width
            && state.ball.y >= state.right_pad.y && state.ball.y <= state.right_pad.y + paddle_height)
        {
            const auto hit_pos = (state.ball.y - (state.right_pad.y + (paddle_height * half))) / (paddle_height * half);
            const auto angle = hit_pos * max_bounce_angle * deg2rad;
            const auto speed = boosted_speed(state.ball.vx, state.ball.vy);
            state.ball.vx = -speed * std::cos(angle);
            state.ball.vy = speed * std::sin(angle);
            state.ball.x = state.right_pad.x - ball_radius;
        }

        // Scoring.
        if (state.ball.x - ball_radius > screen.w)
        {
            ++state.left_pad.score;
            reset_ball(state.ball, -1, screen);
        }

        if (state.ball.x + ball_radius < 0.0F)
        {
            ++state.right_pad.score;
            reset_ball(state.ball, 1, screen);
        }
    }
}

auto main() -> int
{
    try
    {
        using namespace awn::graphics;
        using namespace awn::scene;

        auto engine = awn::Engine{"Awen - Pong", init_width, init_height, {ConfigFlag::resizable, ConfigFlag::high_dpi}};
        awn::Engine::set_target_fps(target_fps);
        engine.set_clear_color(colors::black);

        const auto sw0 = static_cast<float>(init_width);
        const auto sh0 = static_cast<float>(init_height);

        auto state = GameState{
            .left_pad = {.x = paddle_offset, .y = (sh0 * half) - (paddle_height * half), .score = 0},
            .right_pad = {.x = sw0 - paddle_offset - paddle_width, .y = (sh0 * half) - (paddle_height * half), .score = 0},
            .ball = {},
            .p2_ai = true,
        };

        reset_ball(state.ball, 1, ScreenSize{.w = sw0, .h = sh0});

        auto scene = Scene{};
        auto root = scene.root();

        // Dashed center line (z = 0).
        auto dashes = std::vector<NodeHandle<RectNode>>{};
        dashes.reserve(max_dashes);

        for (auto i = 0; i < max_dashes; ++i)
        {
            dashes.push_back(root.add_child<RectNode>().set_size(dash_width, static_cast<float>(dash_height)).set_color(colors::dark_gray));
        }

        // Paddles (z = 1).
        auto left_paddle = root.add_child<RectNode>(1);
        left_paddle.set_size(paddle_width, paddle_height).set_color(colors::white);

        auto right_paddle = root.add_child<RectNode>(1);
        right_paddle.set_size(paddle_width, paddle_height).set_color(colors::white);

        // Ball (z = 1).
        auto ball_node = root.add_child<CircleNode>(1);
        ball_node.set_radius(ball_radius).set_color(colors::white);

        // Scores (z = 2).
        auto left_score = root.add_child<TextNode>(2);
        left_score.set_font_size(score_font_size).set_color(colors::white);

        auto right_score = root.add_child<TextNode>(2);
        right_score.set_font_size(score_font_size).set_color(colors::white);

        // Hints (z = 2).
        auto left_hint = root.add_child<TextNode>(2);
        left_hint.set_text("W / S").set_font_size(hint_font_size).set_color(colors::dark_gray);

        auto right_hint = root.add_child<TextNode>(2);
        right_hint.set_font_size(hint_font_size).set_color(colors::dark_gray);

        engine.on_event(
            [&state](const EventKeyboard& ev)
            {
                if (ev.key == EventKeyboard::Key::space && ev.type == EventKeyboard::Type::pressed)
                {
                    state.p2_ai = !state.p2_ai;
                }
            });

        engine.run(
            scene,
            [&](float dt)
            {
                const auto sw = static_cast<float>(Window::get_screen_width());
                const auto sh = static_cast<float>(Window::get_screen_height());
                const auto screen = ScreenSize{.w = sw, .h = sh};

                state.right_pad.x = sw - paddle_offset - paddle_width;

                // Player 1 input.
                if (Window::is_key_down(EventKeyboard::Key::w))
                {
                    state.left_pad.y -= paddle_speed * dt;
                }

                if (Window::is_key_down(EventKeyboard::Key::s))
                {
                    state.left_pad.y += paddle_speed * dt;
                }

                clamp_paddle(state.left_pad, sh);

                // Player 2 input.
                if (state.p2_ai)
                {
                    update_ai(state.right_pad, state.ball, screen, dt);
                }
                else
                {
                    if (Window::is_key_down(EventKeyboard::Key::up))
                    {
                        state.right_pad.y -= paddle_speed * dt;
                    }

                    if (Window::is_key_down(EventKeyboard::Key::down))
                    {
                        state.right_pad.y += paddle_speed * dt;
                    }

                    clamp_paddle(state.right_pad, sh);
                }

                update_physics(state, screen, dt);

                // Update scene nodes.
                const auto half_w = sw * half;
                const auto screen_w_i = static_cast<int>(sw);
                const auto screen_h_i = static_cast<int>(sh);

                // Dashes.
                auto dash_idx = 0;

                for (auto y = 0; y < screen_h_i && dash_idx < max_dashes; y += dash_gap, ++dash_idx)
                {
                    dashes[static_cast<std::size_t>(dash_idx)].set_transform(Transform{.x = half_w - dash_center_offset, .y = static_cast<float>(y)});
                }

                for (; dash_idx < max_dashes; ++dash_idx)
                {
                    dashes[static_cast<std::size_t>(dash_idx)].set_transform(Transform{.x = -dash_width, .y = -static_cast<float>(dash_height)});
                }

                // Paddles.
                left_paddle.set_transform(Transform{.x = state.left_pad.x, .y = state.left_pad.y});
                right_paddle.set_transform(Transform{.x = state.right_pad.x, .y = state.right_pad.y});

                // Ball.
                ball_node.set_transform(Transform{.x = state.ball.x, .y = state.ball.y});

                // Scores.
                const auto left_score_str = std::to_string(state.left_pad.score);
                const auto right_score_str = std::to_string(state.right_pad.score);

                left_score.set_text(left_score_str)
                    .set_transform(Transform{.x = static_cast<float>(static_cast<int>(half_w) - score_x_left), .y = static_cast<float>(score_y)});

                right_score.set_text(right_score_str)
                    .set_transform(Transform{
                        .x = static_cast<float>(static_cast<int>(half_w) + score_x_left
                                                - Renderer::measure_text(right_score_str.c_str(), score_font_size)),
                        .y = static_cast<float>(score_y),
                    });

                // Hints.
                left_hint.set_transform(Transform{.x = static_cast<float>(hint_x), .y = static_cast<float>(screen_h_i - hint_y_from_bottom)});

                const auto* p2_text = state.p2_ai ? "P2: AI  [SPACE]" : "UP/DOWN  [SPACE]";

                right_hint.set_text(p2_text).set_transform(Transform{
                    .x = static_cast<float>(screen_w_i - Renderer::measure_text(p2_text, hint_font_size) - hint_x),
                    .y = static_cast<float>(screen_h_i - hint_y_from_bottom),
                });
            });

        return EXIT_SUCCESS;
    }
    catch (...)
    {
        return EXIT_FAILURE;
    }
}
