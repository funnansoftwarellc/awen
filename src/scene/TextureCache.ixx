module;

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>

export module awen.scene.texture_cache;

export import awen.scene.texture_id;

export namespace awen::scene
{
    /// @brief Owns Texture2D lifetimes, keyed by file path.
    ///
    /// Textures are loaded on first request and unloaded when the cache is
    /// destroyed. Duplicate load() calls for the same path return the existing
    /// TextureId without reloading.
    class TextureCache
    {
    public:
        TextureCache() = default;

        /// @brief Unloads all managed textures via UnloadTexture.
        ~TextureCache()
        {
            // Skip slot 0 (null sentinel).
            for (auto i = std::size_t{1}; i < std::size(textures_); ++i)
            {
                UnloadTexture(textures_[i]);
            }
        }

        // Non-copyable; textures are unique GPU resources.
        TextureCache(const TextureCache&) = delete;
        auto operator=(const TextureCache&) -> TextureCache& = delete;

        TextureCache(TextureCache&&) = default;
        auto operator=(TextureCache&&) -> TextureCache& = default;

        /// @brief Loads a texture from @p path, or returns the cached TextureId if already loaded.
        /// @param path File path of the image to load.
        /// @return A TextureId that can be passed to get().
        [[nodiscard]] auto load(const std::string& path) -> TextureId
        {
            const auto it = path_to_id_.find(path);

            if (it != path_to_id_.end())
            {
                return it->second;
            }

            if (std::empty(textures_))
            {
                // Slot 0 is reserved as the null sentinel.
                textures_.push_back(Texture2D{});
            }

            const auto index = static_cast<uint32_t>(std::size(textures_));
            textures_.push_back(LoadTexture(path.c_str()));

            const auto id = TextureId{.index = index};
            path_to_id_.emplace(path, id);

            return id;
        }

        /// @brief Returns a const pointer to the Texture2D for @p id, or nullptr if invalid.
        /// @param id TextureId returned by a previous call to load().
        /// @return Const pointer to the Texture2D, or nullptr if id is null or out of range.
        [[nodiscard]] auto get(TextureId id) const -> const Texture2D*
        {
            if (!id.is_valid() || id.index >= static_cast<uint32_t>(std::size(textures_)))
            {
                return nullptr;
            }

            return &textures_[id.index];
        }

    private:
        // textures_[0] is a dummy null sentinel; valid textures start at index 1.
        std::vector<Texture2D> textures_;
        std::unordered_map<std::string, TextureId> path_to_id_;
    };
}
