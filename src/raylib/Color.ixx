module;

#include <raylib.h>
#include <compare>
#include <cstdint>

export module awen.raylib.color;

export namespace awen::raylib
{
    struct Color
    {
        std::uint8_t r{};
        std::uint8_t g{};
        std::uint8_t b{};
        std::uint8_t a{};

        auto operator<=>(const Color&) const noexcept = default;
    };

    namespace colors
    {
        inline constexpr Color White{.r = 255, .g = 255, .b = 255, .a = 255};
        inline constexpr Color Black{.r = 0, .g = 0, .b = 0, .a = 255};
        inline constexpr Color Red{.r = 255, .g = 0, .b = 0, .a = 255};
        inline constexpr Color Green{.r = 0, .g = 255, .b = 0, .a = 255};
        inline constexpr Color Blue{.r = 0, .g = 0, .b = 255, .a = 255};
        inline constexpr Color Yellow{.r = 255, .g = 255, .b = 0, .a = 255};
        inline constexpr Color Magenta{.r = 255, .g = 0, .b = 255, .a = 255};
        inline constexpr Color Cyan{.r = 0, .g = 255, .b = 255, .a = 255};
        inline constexpr Color Orange{.r = 255, .g = 127, .b = 0, .a = 255};
    }

    auto ToRaylibColor(const awen::raylib::Color& color) -> ::Color
    {
        return ::Color{
            .r = color.r,
            .g = color.g,
            .b = color.b,
            .a = color.a,
        };
    }
}