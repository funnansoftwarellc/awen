module;

#include <compare>
#include <cstddef>
#include <functional>
#include <string>

export module awen.sdl.font;

export namespace awen::sdl
{
    /// @brief Identifies a TTF font cached by the renderer.
    ///
    /// An empty `path` selects the renderer's default fallback font.
    struct FontHandle
    {
        std::string path;
        int sizePx{};

        auto operator==(const FontHandle&) const noexcept -> bool = default;
    };

    /// @brief Hasher for FontHandle suitable for `std::unordered_map`.
    struct FontHandleHash
    {
        auto operator()(const FontHandle& handle) const noexcept -> std::size_t
        {
            const auto h1 = std::hash<std::string>{}(handle.path);
            const auto h2 = std::hash<int>{}(handle.sizePx);
            return h1 ^ (h2 + 0x9E3779B9 + (h1 << 6) + (h1 >> 2));
        }
    };
}
