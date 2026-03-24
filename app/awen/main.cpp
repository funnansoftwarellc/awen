#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

#include <raylib.h>

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
    constexpr auto score_x_right = 22;
    constexpr auto score_y = 18;
    constexpr auto score_font_size = 52;
    constexpr auto hint_x = 10;
    constexpr auto hint_y_from_bottom = 22;
    constexpr auto hint_font_size = 16;
    constexpr auto hint_ai_x_offset = 138;
    constexpr auto hint_human_x_offset = 150;

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

    auto clamp_paddle(Paddle& p, float screen_h) -> void
    {
        p.y = std::max(0.0F, p.y);
        p.y = std::min(screen_h - paddle_height, p.y);
    }

    auto reset_ball(Ball& ball, int direction, Vector2 screen_size) -> void
    {
        ball.x = screen_size.x * half;
        ball.y = screen_size.y * half;
        const auto angle = static_cast<float>(GetRandomValue(-ball_launch_angle, ball_launch_angle)) * DEG2RAD;
        ball.vx = static_cast<float>(direction) * ball_init_speed * std::cos(angle);
        ball.vy = ball_init_speed * std::sin(angle);
    }

    auto boosted_speed(float vx, float vy) -> float
    {
        const auto s = std::sqrt((vx * vx) + (vy * vy)) * ball_speed_up;
        return std::min(s, ball_max_speed);
    }

    auto update_ai(Paddle& paddle, const Ball& ball, Vector2 screen_size, float dt) -> void
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
        clamp_paddle(paddle, screen_size.y);
    }

    auto handle_input(GameState& state, Vector2 screen_size, float dt) -> void
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            state.p2_ai = !state.p2_ai;
        }

        if (IsKeyDown(KEY_W))
        {
            state.left_pad.y -= paddle_speed * dt;
        }
        if (IsKeyDown(KEY_S))
        {
            state.left_pad.y += paddle_speed * dt;
        }
        clamp_paddle(state.left_pad, screen_size.y);

        if (state.p2_ai)
        {
            update_ai(state.right_pad, state.ball, screen_size, dt);
        }
        else
        {
            if (IsKeyDown(KEY_UP))
            {
                state.right_pad.y -= paddle_speed * dt;
            }
            if (IsKeyDown(KEY_DOWN))
            {
                state.right_pad.y += paddle_speed * dt;
            }
            clamp_paddle(state.right_pad, screen_size.y);
        }
    }

    auto update_physics(GameState& state, Vector2 screen_size, float dt) -> void
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
        if (state.ball.y + ball_radius > screen_size.y)
        {
            state.ball.y = screen_size.y - ball_radius;
            state.ball.vy = -std::abs(state.ball.vy);
        }

        // Left paddle collision
        if (state.ball.vx < 0.0F && state.ball.x - ball_radius <= state.left_pad.x + paddle_width && state.ball.x - ball_radius >= state.left_pad.x
            && state.ball.y >= state.left_pad.y && state.ball.y <= state.left_pad.y + paddle_height)
        {
            const auto hit_pos = (state.ball.y - (state.left_pad.y + (paddle_height * half))) / (paddle_height * half);
            const auto angle = hit_pos * max_bounce_angle * DEG2RAD;
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
            const auto angle = hit_pos * max_bounce_angle * DEG2RAD;
            const auto speed = boosted_speed(state.ball.vx, state.ball.vy);
            state.ball.vx = -speed * std::cos(angle);
            state.ball.vy = speed * std::sin(angle);
            state.ball.x = state.right_pad.x - ball_radius;
        }

        // Scoring
        if (state.ball.x - ball_radius > screen_size.x)
        {
            ++state.left_pad.score;
            reset_ball(state.ball, -1, screen_size);
        }
        if (state.ball.x + ball_radius < 0.0F)
        {
            ++state.right_pad.score;
            reset_ball(state.ball, 1, screen_size);
        }
    }

    auto draw_game(const GameState& state) -> void
    {
        BeginDrawing();
        ClearBackground(BLACK);

        const auto screen_w = GetScreenWidth();
        const auto screen_h = GetScreenHeight();

        // Dashed center line
        for (int y = 0; y < screen_h; y += dash_gap)
        {
            DrawRectangle((screen_w / 2) - 2, y, 4, dash_height, DARKGRAY);
        }

        DrawRectangleV({state.left_pad.x, state.left_pad.y}, {paddle_width, paddle_height}, WHITE);
        DrawRectangleV({state.right_pad.x, state.right_pad.y}, {paddle_width, paddle_height}, WHITE);

        DrawCircleV({state.ball.x, state.ball.y}, ball_radius, WHITE);

        DrawText(std::to_string(state.left_pad.score).c_str(), (screen_w / 2) - score_x_left, score_y, score_font_size, WHITE);
        DrawText(std::to_string(state.right_pad.score).c_str(), (screen_w / 2) + score_x_right, score_y, score_font_size, WHITE);

        DrawText("W / S", hint_x, screen_h - hint_y_from_bottom, hint_font_size, DARKGRAY);
        if (state.p2_ai)
        {
            DrawText("P2: AI  [SPACE]", screen_w - hint_ai_x_offset, screen_h - hint_y_from_bottom, hint_font_size, DARKGRAY);
        }
        else
        {
            DrawText("UP/DOWN  [SPACE]", screen_w - hint_human_x_offset, screen_h - hint_y_from_bottom, hint_font_size, DARKGRAY);
        }

        EndDrawing();
    }
}

auto main() -> int
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(init_width, init_height, "Awen - Pong");
    SetTargetFPS(target_fps);

    const auto sw0 = static_cast<float>(init_width);
    const auto sh0 = static_cast<float>(init_height);

    GameState state{
        .left_pad = {.x = paddle_offset, .y = (sh0 * half) - (paddle_height * half), .score = 0},
        .right_pad = {.x = sw0 - paddle_offset - paddle_width, .y = (sh0 * half) - (paddle_height * half), .score = 0},
        .ball = {},
        .p2_ai = true,
    };

    reset_ball(state.ball, 1, {sw0, sh0});

    while (!WindowShouldClose())
    {
        const auto sw = static_cast<float>(GetScreenWidth());
        const auto sh = static_cast<float>(GetScreenHeight());
        const auto dt = GetFrameTime();

        state.right_pad.x = sw - paddle_offset - paddle_width;

        handle_input(state, {sw, sh}, dt);
        update_physics(state, {sw, sh}, dt);
        draw_game(state);
    }

    CloseWindow();
    return EXIT_SUCCESS;
}
