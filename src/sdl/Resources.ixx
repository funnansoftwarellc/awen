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
    /// @param path Filesystem path to the .ttf file.
    /// @param pointSize Font point size.
    /// @return Entity carrying a Font component, or an invalid entity on failure.
    auto loadFont(flecs::world& world, std::string_view path, int pointSize) -> flecs::entity
    {
        auto* handle = TTF_OpenFont(std::string{path}.c_str(), static_cast<float>(pointSize));

        if (handle == nullptr)
        {
            return flecs::entity{};
        }

        auto entity = world.entity();

        entity.set<Font>({
            .handle = handle,
            .pointSize = pointSize,
            .path = std::string{path},
        });

        return entity;
    }
}
