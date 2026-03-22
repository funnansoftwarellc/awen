#include <cstdlib>

#include <SDL3/SDL.h>

// NOLINTBEGIN

namespace
{

    constexpr int screenWidth = 800;
    constexpr int screenHeight = 450;

    struct AppState
    {
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        bool running = true;
    };

    void render_frame(void *userdata)
    {
        auto *app = static_cast<AppState *>(userdata);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                app->running = false;
            }
        }

        // ~RAYWHITE background
        SDL_SetRenderDrawColor(app->renderer, 245, 245, 245, 255);
        SDL_RenderClear(app->renderer);

        const SDL_Vertex verts[3] = {
            {{screenWidth / 2.0f, 50.0f}, {255, 0, 0, 255}, {0.0f, 0.0f}},
            {{screenWidth / 2.0f - 150.0f, screenHeight - 100.0f}, {255, 0, 0, 255}, {0.0f, 0.0f}},
            {{screenWidth / 2.0f + 150.0f, screenHeight - 100.0f}, {255, 0, 0, 255}, {0.0f, 0.0f}},
        };
        SDL_RenderGeometry(app->renderer, nullptr, verts, 3, nullptr, 0);

        // ~DARKGRAY text using SDL3's built-in debug font
        SDL_SetRenderDrawColor(app->renderer, 80, 80, 80, 255);
        SDL_RenderDebugText(app->renderer, screenWidth / 2.0f - 54.0f, 20.0f, "Hello, Awen!");

        SDL_RenderPresent(app->renderer);
    }

} // namespace

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    AppState app;
    app.window = SDL_CreateWindow("Hello Awen - Triangle", screenWidth, screenHeight, 0);
    app.renderer = SDL_CreateRenderer(app.window, nullptr);

    while (app.running)
        render_frame(&app);

    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

// NOLINTEND