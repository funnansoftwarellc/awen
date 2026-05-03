module;

#include <SDL3/SDL.h>
#include <awen/flecs/Flecs.hpp>
#include <variant>
#include <vector>

export module awen.sdl.ecs.renderer;
export import awen.sdl.ecs.components;
export import awen.core.overloaded;

export namespace awen::sdl::ecs
{
    class Renderer
    {
    public:
        void sort(flecs::world&)
        {
        }

        void build(flecs::world& world)
        {
            drawables_.clear();
            world.each<Drawable>([this](flecs::entity, const Drawable& drawable) { drawables_.emplace_back(drawable); });
        }

        void draw(SDL_Renderer* renderer)
        {
            for (const auto& drawable : drawables_)
            {
                std::visit(awen::core::Overloaded{[renderer](const ColorRectangle& x)
                                                  {
                                                      SDL_SetRenderDrawColor(renderer, x.color.r, x.color.g, x.color.b, x.color.a);
                                                      SDL_FRect rect{.x = x.x, .y = x.y, .w = x.width, .h = x.height};
                                                      SDL_RenderFillRect(renderer, &rect);
                                                  },
                                                  [](const auto&) { /* Unhandled drawable type */ }},
                           drawable);
            }
        }

    private:
        std::vector<Drawable> drawables_{};
    };
}