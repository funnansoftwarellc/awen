#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <random>
#include <string>

import awen.graphics;

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
    constexpr auto dash_gap = 20;
    constexpr auto dash_height = 10;
    constexpr auto score_x_left = 70;
    constexpr auto score_y = 18;
    constexpr auto score_font_size = 52;
    constexpr auto hint_x = 10;
    constexpr auto hint_y_from_bottom = 22;
    constexpr auto hint_font_size = 16;
    constexpr auto deg2rad = std::numbers::pi_v<float> / 180.0F;

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

    auto reset_ball(Ball& ball, int direction, float screen_w, float screen_h) -> void
    {
        ball.x = screen_w * half;
        ball.y = screen_h * half;
        const auto angle = static_cast<float>(random_value(-ball_launch_angle, ball_launch_angle)) * deg2rad;
        ball.vx = static_cast<float>(direction) * ball_init_speed * std::cos(angle);
        ball.vy = ball_init_speed * std::sin(angle);
    }

    auto boosted_speed(float vx, float vy) -> float
    {
        const auto s = std::sqrt((vx * vx) + (vy * vy)) * ball_speed_up;
        return std::min(s, ball_max_speed);
    }

    auto update_ai(Paddle& paddle, const Ball& ball, float screen_h, float dt) -> void
    {
        const auto center = paddle.y + (paddle_height * half);
        const auto diff = ball.y - center;
        const auto move = paddle_speed * ai_speed_ratio * dt;
        if (std::abs(diff) < move)
        {
            paddle.y += diff;
        }
        else
        {
            paddle.y += (diff > 0.0F) ? move : -move;
        }
        clamp_paddle(paddle, screen_h);
    }

    auto update_physics(GameState& state, float screen_w, float screen_h, float dt) -> void
    {
        state.ball.x += state.ball.vx * dt;
        state.ball.y += state.ball.vy * dt;

        // Wall collision top
        if (state.ball.y - ball_radius < 0.0F)
        {
            state.ball.y = ball_radius;
            state.ball.vy = std::abs(state.ball.vy);
        }
        // Wall collision bottom
        if (state.ball.y + ball_radius > screen_h)
        {
            state.ball.y = screen_h - ball_radius;
            state.ball.vy = -std::abs(state.ball.vy);
        }

        // Left paddle collision
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

        // Right paddle collision
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

        // Scoring
        if (state.ball.x - ball_radius > screen_w)
        {
            ++state.left_pad.score;
            reset_ball(state.ball, -1, screen_w, screen_h);
        }
        if (state.ball.x + ball_radius < 0.0F)
        {
            ++state.right_pad.score;
            reset_ball(state.ball, 1, screen_w, screen_h);
        }
    }
}

auto main() -> int
{
    using namespace awn::graphics;

    auto window = Window{"Awen - Pong", init_width, init_height, {ConfigFlag::resizable}};
    window.set_target_fps(target_fps);

    const auto sw0 = static_cast<float>(init_width);
    const auto sh0 = static_cast<float>(init_height);

    auto state = GameState{
        .left_pad = {.x = paddle_offset, .y = (sh0 * half) - (paddle_height * half), .score = 0},
        .right_pad = {.x = sw0 - paddle_offset - paddle_width, .y = (sh0 * half) - (paddle_height * half), .score = 0},
        .ball = {},
        .p2_ai = true,
    };

    reset_ball(state.ball, 1, sw0, sh0);

    window.on_event(
        [&state](const EventKeyboard& ev)
        {
            if (ev.key == EventKeyboard::Key::space && ev.type == EventKeyboard::Type::pressed)
            {
                state.p2_ai = !state.p2_ai;
            }
        });

    auto renderer = Renderer{};

    while (window.is_open())
    {
        window.poll_events();

        const auto sw = static_cast<float>(window.get_screen_width());
        const auto sh = static_cast<float>(window.get_screen_height());
        const auto dt = window.get_frame_time();

        state.right_pad.x = sw - paddle_offset - paddle_width;

        // Player 1 input
        if (window.is_key_down(EventKeyboard::Key::w))
        {
            state.left_pad.y -= paddle_speed * dt;
        }
        if (window.is_key_down(EventKeyboard::Key::s))
        {
            state.left_pad.y += paddle_speed * dt;
        }
        clamp_paddle(state.left_pad, sh);

        // Player 2 input
        if (state.p2_ai)
        {
            update_ai(state.right_pad, state.ball, sh, dt);
        }
        else
        {
            if (window.is_key_down(EventKeyboard::Key::up))
            {
                state.right_pad.y -= paddle_speed * dt;
            }
            if (window.is_key_down(EventKeyboard::Key::down))
            {
                state.right_pad.y += paddle_speed * dt;
            }
            clamp_paddle(state.right_pad, sh);
        }

        update_physics(state, sw, sh, dt);

        // Draw
        renderer.begin();
        renderer.clear(colors::black);

        const auto screen_w = window.get_screen_width();
        const auto screen_h = window.get_screen_height();

        // Dashed center line
        for (auto y = 0; y < screen_h; y += dash_gap)
        {
            renderer.draw_rect(static_cast<float>((screen_w / 2) - 2), static_cast<float>(y), 4.0F, static_cast<float>(dash_height),
                               colors::dark_gray);
        }

        // Paddles
        renderer.draw_rect(state.left_pad.x, state.left_pad.y, paddle_width, paddle_height, colors::white);
        renderer.draw_rect(state.right_pad.x, state.right_pad.y, paddle_width, paddle_height, colors::white);

        // Ball
        renderer.draw_circle(state.ball.x, state.ball.y, ball_radius, colors::white);

        // Scores
        const auto left_score_str = std::to_string(state.left_pad.score);
        const auto right_score_str = std::to_string(state.right_pad.score);
        renderer.draw_text(left_score_str.c_str(), (screen_w / 2) - score_x_left, score_y, score_font_size, colors::white);
        renderer.draw_text(right_score_str.c_str(), (screen_w / 2) + score_x_left - renderer.measure_text(right_score_str.c_str(), score_font_size),
                           score_y, score_font_size, colors::white);

        // Hints
        renderer.draw_text("W / S", hint_x, screen_h - hint_y_from_bottom, hint_font_size, colors::dark_gray);
        const auto* p2_text = state.p2_ai ? "P2: AI  [SPACE]" : "UP/DOWN  [SPACE]";
        renderer.draw_text(p2_text, screen_w - renderer.measure_text(p2_text, hint_font_size) - hint_x, screen_h - hint_y_from_bottom, hint_font_size,
                           colors::dark_gray);

        renderer.end();
    }

    return EXIT_SUCCESS;
}
