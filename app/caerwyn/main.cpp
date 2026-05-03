#include <print>

#include <SDL3/SDL.h>

auto main() -> int
try
{
    constexpr auto width = 1080;
    constexpr auto height = 1920;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return EXIT_FAILURE;
    }

    auto flags = SDL_WindowFlags{};
    flags |= SDL_WINDOW_RESIZABLE;
    flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

    auto* window = SDL_CreateWindow("Caerwyn", width, height, flags);

    auto* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL, true, "Caerwyn");

    auto* renderer = SDL_CreateGPURenderer(device, window);

    auto running = true;
    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}