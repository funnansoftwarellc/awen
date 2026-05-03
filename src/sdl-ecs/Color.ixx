module;

#include <compare>
#include <cstdint>
#include <limits>

export module awen.sdl.ecs.color;

export namespace awen::sdl::ecs
{
    struct Color
    {
        std::uint8_t r{};
        std::uint8_t g{};
        std::uint8_t b{};
        std::uint8_t a{std::numeric_limits<std::uint8_t>::max()};

        auto operator<=>(const Color&) const noexcept = default;
    };

    namespace colors
    {
        inline constexpr auto Black = Color{.r = 0, .g = 0, .b = 0};
        inline constexpr auto White = Color{.r = 255, .g = 255, .b = 255};
        inline constexpr auto Red = Color{.r = 230, .g = 41, .b = 55};
        inline constexpr auto Green = Color{.r = 0, .g = 228, .b = 48};
        inline constexpr auto Blue = Color{.r = 0, .g = 121, .b = 241};
        inline constexpr auto Yellow = Color{.r = 253, .g = 249, .b = 0};
        inline constexpr auto Orange = Color{.r = 255, .g = 161, .b = 0};
        inline constexpr auto Purple = Color{.r = 200, .g = 122, .b = 255};
        inline constexpr auto DarkGray = Color{.r = 80, .g = 80, .b = 80};
        inline constexpr auto LightGray = Color{.r = 200, .g = 200, .b = 200};
        inline constexpr auto Gray = Color{.r = 130, .g = 130, .b = 130};
    }
}