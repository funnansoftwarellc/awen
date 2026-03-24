module;

#include <compare>
#include <cstdint>

export module awen.scene.texture_id;

export namespace awn::scene
{
    /// @brief Opaque handle to a texture managed by TextureCache.
    ///
    /// Index 0 is the null sentinel; all valid handles have index >= 1.
    struct TextureId
    {
        uint32_t index{};

        /// @brief Returns true when the handle refers to a potentially live texture.
        [[nodiscard]] auto is_valid() const noexcept -> bool
        {
            return index != 0;
        }

        auto operator<=>(const TextureId&) const noexcept = default;
    };

    /// @brief Sentinel value representing an absent or unset texture reference.
    inline constexpr auto null_texture = TextureId{};
}
