module;

#include <SDL3/SDL.h>
#include <glm/vec2.hpp>
#include <string>
#include <variant>

export module awen.sdl.ecs.components;
export import awen.sdl.ecs.color;
export import awen.core.enummask;

export namespace awen::sdl::ecs
{
    struct Window
    {
        enum class Flags : uint32_t
        {
            Resizable = SDL_WINDOW_RESIZABLE,
            HighPixelDensity = SDL_WINDOW_HIGH_PIXEL_DENSITY
        };

        std::string title;
        Color color{colors::Black};
        awen::core::EnumMask<Flags> flags{};
        int x{};
        int y{};
        int width{};
        int height{};
        bool running{false};
    };

    struct Transform
    {
        glm::vec2 position{};
        glm::vec2 scale{1.0F, 1.0F};
        float rotation{};
    };

    struct ColorRectangle
    {
        Color color{colors::White};
        float x{};
        float y{};
        float width{};
        float height{};
    };

    using Drawable = std::variant<ColorRectangle>;
}