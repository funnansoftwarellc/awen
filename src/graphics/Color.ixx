module;

#include <cstdint>
#include <limits>

export module awen.graphics.color;

export namespace awn::graphics
{
    struct Color
    {
        std::uint8_t r{};
        std::uint8_t g{};
        std::uint8_t b{};
        std::uint8_t a{std::numeric_limits<std::uint8_t>::max()};
    };

    namespace colors
    {
        inline constexpr auto black = Color{.r = 0, .g = 0, .b = 0};
        inline constexpr auto white = Color{.r = 255, .g = 255, .b = 255};
        inline constexpr auto red = Color{.r = 230, .g = 41, .b = 55};
        inline constexpr auto green = Color{.r = 0, .g = 228, .b = 48};
        inline constexpr auto blue = Color{.r = 0, .g = 121, .b = 241};
        inline constexpr auto yellow = Color{.r = 253, .g = 249, .b = 0};
        inline constexpr auto orange = Color{.r = 255, .g = 161, .b = 0};
        inline constexpr auto purple = Color{.r = 200, .g = 122, .b = 255};
        inline constexpr auto dark_gray = Color{.r = 80, .g = 80, .b = 80};
        inline constexpr auto light_gray = Color{.r = 200, .g = 200, .b = 200};
        inline constexpr auto gray = Color{.r = 130, .g = 130, .b = 130};
    }
}
