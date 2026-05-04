module;

#include <SDL3/SDL.h>
#include <cstdint>
#include <string>

export module awen.sdl.components;

export import awen.sdl.color;
export import awen.core.enummask;

export namespace awen::sdl
{
    /// @brief Window component. Setting it on an entity creates the SDL window
    ///        and renderer; removing it tears them down.
    struct Window
    {
        enum class Flags : std::uint32_t
        {
            Resizable = SDL_WINDOW_RESIZABLE,
            HighPixelDensity = SDL_WINDOW_HIGH_PIXEL_DENSITY,
        };

        std::string title;
        Color color{colors::Black};
        awen::core::EnumMask<Flags> flags{};
        int x{};
        int y{};
        int width{};
        int height{};
    };

    /// @brief Internal component that pairs a Window entity with its native handles.
    /// @note Set automatically by the Window OnSet observer.
    struct WindowHandles
    {
        SDL_Window* window{};
        SDL_Renderer* renderer{};
    };
}
