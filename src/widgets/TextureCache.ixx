module;

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>

export module awen.widgets.texture_cache;

export import awen.graphics.texture_id;

export namespace awn::widgets
{
    /// @brief Owns Texture2D lifetimes, keyed by file path.
    class TextureCache
    {
    public:
        TextureCache() = default;

        /// @brief Unloads all managed textures via UnloadTexture.
        ~TextureCache()
        {
            for (auto i = std::size_t{1}; i < std::size(textures_); ++i)
            {
                UnloadTexture(textures_[i]);
            }
        }

        TextureCache(const TextureCache&) = delete;
        auto operator=(const TextureCache&) -> TextureCache& = delete;

        TextureCache(TextureCache&&) = default;
        auto operator=(TextureCache&&) -> TextureCache& = default;

        /// @brief Loads a texture from @p path or returns the cached TextureId.
        [[nodiscard]] auto load(const std::string& path) -> awn::graphics::TextureId
        {
            const auto it = path_to_id_.find(path);

            if (it != path_to_id_.end())
            {
                return it->second;
            }

            if (std::empty(textures_))
            {
                textures_.push_back(Texture2D{});
            }

            const auto index = static_cast<uint32_t>(std::size(textures_));
            textures_.push_back(LoadTexture(path.c_str()));

            const auto id = awn::graphics::TextureId{.index = index};
            path_to_id_.emplace(path, id);

            return id;
        }

        /// @brief Returns a pointer to the Texture2D for @p id, or nullptr if invalid.
        [[nodiscard]] auto get(awn::graphics::TextureId id) const -> const Texture2D*
        {
            if (!id.is_valid() || id.index >= static_cast<uint32_t>(std::size(textures_)))
            {
                return nullptr;
            }

            return &textures_[id.index];
        }

    private:
        std::vector<Texture2D> textures_;
        std::unordered_map<std::string, awn::graphics::TextureId> path_to_id_;
    };
}
