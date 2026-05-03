module;

#include <SDL3/SDL.h>
#include <awen/flecs/Flecs.hpp>
#include <optional>

export module awen.sdl.ecs;
export import awen.sdl.ecs.renderer;
export import awen.sdl.ecs.components;

export namespace awen::sdl::ecs
{
    class Module
    {
    public:
        explicit Module(flecs::world& world)
        {
            world.observer<Window>()
                .event(flecs::OnSet)
                .each(
                    [this](flecs::entity, Window& x)
                    {
                        auto flags = SDL_WindowFlags{};

                        if (x.flags.test(Window::Flags::Resizable))
                        {
                            flags |= SDL_WINDOW_RESIZABLE;
                        }

                        if (x.flags.test(Window::Flags::HighPixelDensity))
                        {
                            flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
                        }

                        window_ = SDL_CreateWindow(x.title.c_str(), x.width, x.height, flags);
                        sdlRenderer_ = SDL_CreateGPURenderer(nullptr, window_);
                        renderer_.emplace();

                        x.running = window_ && sdlRenderer_;
                    });

            world.observer<Window>()
                .event(flecs::OnRemove)
                .each(
                    [this](flecs::entity, const Window&)
                    {
                        SDL_DestroyRenderer(sdlRenderer_);
                        SDL_DestroyWindow(window_);
                        SDL_Quit();
                    });

            world.system<Window>().each(
                [this, &world](flecs::entity, Window& w)
                {
                    SDL_Event event;

                    while (SDL_PollEvent(&event))
                    {
                        switch (event.type)
                        {
                            case SDL_EVENT_QUIT:
                                w.running = false;
                                break;

                            case SDL_EVENT_KEY_DOWN:
                                if (event.key.key == SDLK_ESCAPE)
                                {
                                    w.running = false;
                                }
                                break;
                        }
                    }

                    // Begin
                    SDL_SetRenderDrawColor(sdlRenderer_, w.color.r, w.color.g, w.color.b, w.color.a);
                    SDL_RenderClear(sdlRenderer_);

                    if (renderer_)
                    {
                        renderer_->sort(world);
                        renderer_->build(world);
                        renderer_->draw(sdlRenderer_);
                    }

                    // End
                    SDL_RenderPresent(sdlRenderer_);
                });
        }

    private:
        SDL_Window* window_{};
        SDL_Renderer* sdlRenderer_{};
        std::optional<Renderer> renderer_{};
    };
}