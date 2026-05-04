module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <awen/flecs/Flecs.hpp>
#include <string>
#include <string_view>

export module awen.sdl.resources;

export namespace awen::sdl
{
    /// @brief Loaded TTF font handle. Owned by the entity carrying it; an
    ///        OnRemove observer (registered by the Module) closes the font.
    struct Font
    {
        TTF_Font* handle{};
        int pointSize{};
        std::string path;
    };

    /// @brief GPU texture handle. Owned by the entity carrying it; an
    ///        OnRemove observer (registered by the Module) destroys it.
    struct Texture
    {
        SDL_Texture* handle{};
        int width{};
        int height{};
        std::string path;
    };

    /// @brief Load a TTF font as a resource entity.
    /// @param world The flecs world.
    /// @param path Filesystem path to the .ttf file. Absolute paths are used
    ///        as-is; relative paths are resolved first against the current
    ///        working directory, and on failure against `SDL_GetBasePath()`
    ///        (the directory containing the executable).
    /// @param pointSize Font point size.
    /// @return Entity carrying a Font component, or an invalid entity on failure.
    auto loadFont(flecs::world& world, std::string_view path, int pointSize) -> flecs::entity
    {
        const auto pointSizeF = static_cast<float>(pointSize);
        auto resolved = std::string{path};
        auto* handle = TTF_OpenFont(resolved.c_str(), pointSizeF);

        if (handle == nullptr)
        {
            const auto* base = SDL_GetBasePath();

            if (base != nullptr)
            {
                auto fallback = std::string{base} + resolved;
                handle = TTF_OpenFont(fallback.c_str(), pointSizeF);

                if (handle != nullptr)
                {
                    resolved = std::move(fallback);
                }
            }
        }

        if (handle == nullptr)
        {
            return flecs::entity{};
        }

        auto entity = world.entity();

        entity.set<Font>({
            .handle = handle,
            .pointSize = pointSize,
            .path = std::move(resolved),
        });

        return entity;
    }
}
