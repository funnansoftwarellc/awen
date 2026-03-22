#include <cstdlib>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// NOLINTBEGIN

namespace
{

    constexpr float kWidth = 800.0f;
    constexpr float kHeight = 450.0f;
    constexpr float kPaddleW = 15.0f;
    constexpr float kPaddleH = 80.0f;
    constexpr float kBallSize = 10.0f;
    constexpr float kPaddleSpeed = 320.0f;
    constexpr float kBallSpeed0 = 240.0f;
    constexpr float kBallSpeedMax = 560.0f;
    constexpr float kAiSpeed = 230.0f;
    constexpr float kMargin = 20.0f;
    constexpr float kResetDelay = 1.2f;

    struct Ball
    {
        float x{kWidth / 2.0f - kBallSize / 2.0f};
        float y{kHeight / 2.0f - kBallSize / 2.0f};
        float vx{kBallSpeed0};
        float vy{80.0f};
    };

    struct AppState
    {
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        bool running = true;

        Ball ball;
        float leftY = kHeight / 2.0f - kPaddleH / 2.0f;
        float rightY = kHeight / 2.0f - kPaddleH / 2.0f;

        int leftScore = 0;
        int rightScore = 0;

        float resetTimer = 0.0f; // >0: ball frozen after a score

        bool hasTouch = false;
        SDL_FingerID touchId = 0;
        float touchGameY = 0.0f;
    };

    void fill_rect(SDL_Renderer *r, float x, float y, float w, float h)
    {
        SDL_FRect rect{x, y, w, h};
        SDL_RenderFillRect(r, &rect);
    }

    void reset_ball(AppState *app, int scorer)
    {
        app->ball.x = kWidth / 2.0f - kBallSize / 2.0f;
        app->ball.y = kHeight / 2.0f - kBallSize / 2.0f;
        // Serve toward the loser so they must react
        float dir = (scorer == 0) ? 1.0f : -1.0f;
        app->ball.vx = dir * kBallSpeed0;
        app->ball.vy = ((app->leftScore + app->rightScore) % 2 == 0) ? 80.0f : -80.0f;
        app->resetTimer = kResetDelay;
    }

    void handle_events(AppState *app)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    app->running = false;
                    break;
                case SDL_EVENT_FINGER_DOWN:
                    app->hasTouch = true;
                    app->touchId = event.tfinger.fingerID;
                    app->touchGameY = event.tfinger.y * kHeight;
                    break;
                case SDL_EVENT_FINGER_MOTION:
                    if (event.tfinger.fingerID == app->touchId)
                        app->touchGameY = event.tfinger.y * kHeight;
                    break;
                case SDL_EVENT_FINGER_UP:
                    if (event.tfinger.fingerID == app->touchId)
                        app->hasTouch = false;
                    break;
                default:
                    break;
            }
        }
    }

    void update(AppState *app, float dt)
    {
        // Left paddle: touch (Android) or keyboard (desktop)
        if (app->hasTouch)
        {
            float target = app->touchGameY - kPaddleH / 2.0f;
            float diff = target - app->leftY;
            float step = kPaddleSpeed * dt;
            app->leftY += (diff > 0.0f) ? SDL_min(diff, step) : SDL_max(diff, -step);
        }
        else
        {
            const bool *keys = SDL_GetKeyboardState(nullptr);
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
                app->leftY -= kPaddleSpeed * dt;
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
                app->leftY += kPaddleSpeed * dt;
        }
        app->leftY = SDL_clamp(app->leftY, 0.0f, kHeight - kPaddleH);

        // Right paddle AI tracks the ball
        float ballMid = app->ball.y + kBallSize / 2.0f;
        float rightMid = app->rightY + kPaddleH / 2.0f;
        float aiDiff = ballMid - rightMid;
        float aiStep = kAiSpeed * dt;
        app->rightY += (aiDiff > 0.0f) ? SDL_min(aiDiff, aiStep) : SDL_max(aiDiff, -aiStep);
        app->rightY = SDL_clamp(app->rightY, 0.0f, kHeight - kPaddleH);

        // Ball is frozen during the post-score pause
        if (app->resetTimer > 0.0f)
        {
            app->resetTimer -= dt;
            return;
        }

        Ball &b = app->ball;
        b.x += b.vx * dt;
        b.y += b.vy * dt;

        // Top / bottom wall bounce
        if (b.y <= 0.0f)
        {
            b.y = 0.0f;
            b.vy = SDL_fabsf(b.vy);
        }
        if (b.y + kBallSize >= kHeight)
        {
            b.y = kHeight - kBallSize;
            b.vy = -SDL_fabsf(b.vy);
        }

        // Left paddle collision
        const float lx = kMargin;
        if (b.vx < 0.0f && b.x <= lx + kPaddleW && b.x >= lx - kBallSize && b.y + kBallSize >= app->leftY && b.y <= app->leftY + kPaddleH)
        {
            float rel = (b.y + kBallSize / 2.0f - (app->leftY + kPaddleH / 2.0f)) / (kPaddleH / 2.0f);
            rel = SDL_clamp(rel, -1.0f, 1.0f);
            float angle = rel * (SDL_PI_F / 3.0f); // up to ±60°
            float speed = SDL_min(SDL_sqrtf(b.vx * b.vx + b.vy * b.vy) * 1.05f, kBallSpeedMax);
            b.vx = SDL_fabsf(SDL_cosf(angle)) * speed;
            b.vy = SDL_sinf(angle) * speed;
            b.x = lx + kPaddleW;
        }

        // Right paddle collision
        const float rx = kWidth - kMargin - kPaddleW;
        if (b.vx > 0.0f && b.x + kBallSize >= rx && b.x <= rx + kPaddleW && b.y + kBallSize >= app->rightY && b.y <= app->rightY + kPaddleH)
        {
            float rel = (b.y + kBallSize / 2.0f - (app->rightY + kPaddleH / 2.0f)) / (kPaddleH / 2.0f);
            rel = SDL_clamp(rel, -1.0f, 1.0f);
            float angle = rel * (SDL_PI_F / 3.0f);
            float speed = SDL_min(SDL_sqrtf(b.vx * b.vx + b.vy * b.vy) * 1.05f, kBallSpeedMax);
            b.vx = -SDL_fabsf(SDL_cosf(angle)) * speed;
            b.vy = SDL_sinf(angle) * speed;
            b.x = rx - kBallSize;
        }

        // Scoring: ball exits left → right scores; exits right → left scores
        if (b.x + kBallSize < 0.0f)
        {
            ++app->rightScore;
            reset_ball(app, 1); // right scored, serve toward left (loser)
        }
        else if (b.x > kWidth)
        {
            ++app->leftScore;
            reset_ball(app, 0); // left scored, serve toward right (loser)
        }
    }

    void render(AppState *app)
    {
        SDL_SetRenderDrawColor(app->renderer, 20, 20, 20, 255);
        SDL_RenderClear(app->renderer);
        SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);

        // Dashed center divider
        for (float y = 8.0f; y < kHeight; y += 20.0f)
            fill_rect(app->renderer, kWidth / 2.0f - 2.0f, y, 4.0f, 12.0f);

        // Paddles
        fill_rect(app->renderer, kMargin, app->leftY, kPaddleW, kPaddleH);
        fill_rect(app->renderer, kWidth - kMargin - kPaddleW, app->rightY, kPaddleW, kPaddleH);

        // Ball
        fill_rect(app->renderer, app->ball.x, app->ball.y, kBallSize, kBallSize);

        // Scores
        char buf[8];
        SDL_snprintf(buf, sizeof(buf), "%d", app->leftScore);
        SDL_RenderDebugText(app->renderer, kWidth / 2.0f - 60.0f, 36.0f, buf);
        SDL_snprintf(buf, sizeof(buf), "%d", app->rightScore);
        SDL_RenderDebugText(app->renderer, kWidth / 2.0f + 50.0f, 36.0f, buf);

        SDL_RenderPresent(app->renderer);
    }

} // namespace

auto main(int /*unused*/, char ** /*unused*/) -> int
{
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "Landscape");

    SDL_Init(SDL_INIT_VIDEO);

    AppState app;
    app.window = SDL_CreateWindow("Pong", static_cast<int>(kWidth), static_cast<int>(kHeight),
#ifdef __ANDROID__
                                  SDL_WINDOW_FULLSCREEN
#else
                                  SDL_WINDOW_RESIZABLE
#endif
    );
    app.renderer = SDL_CreateRenderer(app.window, nullptr);
    SDL_SetRenderLogicalPresentation(app.renderer, static_cast<int>(kWidth), static_cast<int>(kHeight), SDL_LOGICAL_PRESENTATION_STRETCH);

    Uint64 lastTicks = SDL_GetTicks();

    while (app.running)
    {
        const Uint64 now = SDL_GetTicks();
        const float dt = SDL_clamp(static_cast<float>(now - lastTicks) / 1000.0f, 0.0f, 0.05f);
        lastTicks = now;

        handle_events(&app);
        update(&app, dt);
        render(&app);
    }

    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

// NOLINTEND