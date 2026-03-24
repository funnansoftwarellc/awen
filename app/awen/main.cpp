#include <cstdlib>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

#ifdef __APPLE__
#include "include/ports/SkFontMgr_mac_ct.h"
#elifdef __ANDROID__
#include "include/ports/SkFontMgr_android.h"
#include "include/ports/SkFontScanner_FreeType.h"
#elifdef __EMSCRIPTEN__
#include "include/ports/SkFontMgr_data.h"
#elifdef _WIN32
#include "include/ports/SkFontMgr_directory.h"
#else
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"
#endif

#include "gpu_surface.h"

// NOLINTBEGIN

namespace
{

    // ── Game constants ──────────────────────────────────────────────────────

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

    // ── Platform font manager ───────────────────────────────────────────────

    auto create_font_manager() -> sk_sp<SkFontMgr>
    {
#ifdef __APPLE__
        return SkFontMgr_New_CoreText(nullptr);
#elifdef __ANDROID__
        return SkFontMgr_New_Android(nullptr, SkFontScanner_Make_FreeType());
#elifdef __EMSCRIPTEN__
        return SkFontMgr_New_Custom_Empty();
#elifdef _WIN32
        return SkFontMgr_New_Custom_Directory("C:\\Windows\\Fonts");
#else
        return SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
#endif
    }

    // ── Game objects ────────────────────────────────────────────────────────

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
        std::unique_ptr<awen::GpuSurface> gpu;
        bool running = true;

        // Font / text
        sk_sp<SkFontMgr> fontMgr;
        sk_sp<SkTypeface> typeface;
        SkFont scoreFont;

        // Game state
        Ball ball;
        float leftY = kHeight / 2.0f - kPaddleH / 2.0f;
        float rightY = kHeight / 2.0f - kPaddleH / 2.0f;
        int leftScore = 0;
        int rightScore = 0;
        float resetTimer = 0.0f;

        // Touch input (Android)
        bool hasTouch = false;
        SDL_FingerID touchId = 0;
        float touchGameY = 0.0f;
    };

    // ── Game logic ──────────────────────────────────────────────────────────

    void reset_ball(AppState *app, int scorer)
    {
        app->ball.x = kWidth / 2.0f - kBallSize / 2.0f;
        app->ball.y = kHeight / 2.0f - kBallSize / 2.0f;
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

        float ballMid = app->ball.y + kBallSize / 2.0f;
        float rightMid = app->rightY + kPaddleH / 2.0f;
        float aiDiff = ballMid - rightMid;
        float aiStep = kAiSpeed * dt;
        app->rightY += (aiDiff > 0.0f) ? SDL_min(aiDiff, aiStep) : SDL_max(aiDiff, -aiStep);
        app->rightY = SDL_clamp(app->rightY, 0.0f, kHeight - kPaddleH);

        if (app->resetTimer > 0.0f)
        {
            app->resetTimer -= dt;
            return;
        }

        Ball &b = app->ball;
        b.x += b.vx * dt;
        b.y += b.vy * dt;

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

        const float lx = kMargin;
        if (b.vx < 0.0f && b.x <= lx + kPaddleW && b.x >= lx - kBallSize && b.y + kBallSize >= app->leftY && b.y <= app->leftY + kPaddleH)
        {
            float rel = (b.y + kBallSize / 2.0f - (app->leftY + kPaddleH / 2.0f)) / (kPaddleH / 2.0f);
            rel = SDL_clamp(rel, -1.0f, 1.0f);
            float angle = rel * (SDL_PI_F / 3.0f);
            float speed = SDL_min(SDL_sqrtf(b.vx * b.vx + b.vy * b.vy) * 1.05f, kBallSpeedMax);
            b.vx = SDL_fabsf(SDL_cosf(angle)) * speed;
            b.vy = SDL_sinf(angle) * speed;
            b.x = lx + kPaddleW;
        }

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

        if (b.x + kBallSize < 0.0f)
        {
            ++app->rightScore;
            reset_ball(app, 1);
        }
        else if (b.x > kWidth)
        {
            ++app->leftScore;
            reset_ball(app, 0);
        }
    }

    // ── Render ──────────────────────────────────────────────────────────────

    void render(AppState *app)
    {
        sk_sp<SkSurface> surface = app->gpu->begin_frame();
        if (!surface)
            return;

        SkCanvas *canvas = surface->getCanvas();

        // Scale canvas so game coordinates map to actual drawable size
        int w = surface->width();
        int h = surface->height();
        float sx = static_cast<float>(w) / kWidth;
        float sy = static_cast<float>(h) / kHeight;
        canvas->save();
        canvas->scale(sx, sy);

        // Background
        canvas->clear(SkColorSetARGB(255, 20, 20, 20));

        SkPaint white;
        white.setColor(SK_ColorWHITE);
        white.setAntiAlias(true);

        // Dashed center divider
        for (float y = 8.0f; y < kHeight; y += 20.0f)
            canvas->drawRect(SkRect::MakeXYWH(kWidth / 2.0f - 2.0f, y, 4.0f, 12.0f), white);

        // Paddles
        canvas->drawRect(SkRect::MakeXYWH(kMargin, app->leftY, kPaddleW, kPaddleH), white);
        canvas->drawRect(SkRect::MakeXYWH(kWidth - kMargin - kPaddleW, app->rightY, kPaddleW, kPaddleH), white);

        // Ball
        canvas->drawRect(SkRect::MakeXYWH(app->ball.x, app->ball.y, kBallSize, kBallSize), white);

        // Scores
        char buf[8];
        SDL_snprintf(buf, sizeof(buf), "%d", app->leftScore);
        canvas->drawString(buf, kWidth / 2.0f - 60.0f, 36.0f + 24.0f, app->scoreFont, white);
        SDL_snprintf(buf, sizeof(buf), "%d", app->rightScore);
        canvas->drawString(buf, kWidth / 2.0f + 50.0f, 36.0f + 24.0f, app->scoreFont, white);

        canvas->restore();

        app->gpu->end_frame();
    }

    // ── Initialization ──────────────────────────────────────────────────────

    bool init(AppState *app)
    {
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "Landscape");

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        Uint32 windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#ifdef __APPLE__
        windowFlags |= SDL_WINDOW_METAL;
#elifdef __EMSCRIPTEN__
        // Emscripten uses its own canvas; no extra flag needed.
#else
        windowFlags |= SDL_WINDOW_VULKAN;
#endif

        app->window = SDL_CreateWindow("Pong", static_cast<int>(kWidth), static_cast<int>(kHeight),
#ifdef __ANDROID__
                                       SDL_WINDOW_FULLSCREEN | windowFlags
#else
                                       windowFlags
#endif
        );
        if (!app->window)
        {
            SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
            return false;
        }

        // ── GPU surface (platform-specific backend) ─────────────────────────
        app->gpu = awen::GpuSurface::create(app->window);
        if (!app->gpu)
        {
            SDL_Log("Failed to create GPU surface");
            return false;
        }

        // ── Font setup ──────────────────────────────────────────────────────
        app->fontMgr = create_font_manager();
        app->typeface = app->fontMgr->legacyMakeTypeface("Menlo", SkFontStyle());
        if (!app->typeface)
            app->typeface = app->fontMgr->legacyMakeTypeface(nullptr, SkFontStyle());

        app->scoreFont = SkFont(app->typeface, 24.0f);
        app->scoreFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        app->scoreFont.setSubpixel(true);

        return true;
    }

    // ── Cleanup ─────────────────────────────────────────────────────────────

    void cleanup(AppState *app)
    {
        app->gpu.reset();
        if (app->window)
            SDL_DestroyWindow(app->window);
        SDL_Quit();
    }

} // namespace

auto main(int /*unused*/, char ** /*unused*/) -> int
{
    AppState app;
    if (!init(&app))
    {
        cleanup(&app);
        return EXIT_FAILURE;
    }

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

    cleanup(&app);
    return EXIT_SUCCESS;
}

// NOLINTEND