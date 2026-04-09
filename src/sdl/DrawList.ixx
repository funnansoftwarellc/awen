module;

#include <string>
#include <variant>
#include <vector>

#include <SDL3/SDL_render.h>

export module awen.sdl.drawlist;

import awen.sdl.color;

export namespace awen::sdl
{
    struct DrawRectangle
    {
        float x{};
        float y{};
        float width{};
        float height{};
        Color color{};
    };

    struct DrawCircle
    {
        float centerX{};
        float centerY{};
        float radius{};
        Color color{};
    };

    struct DrawText
    {
        std::string text;
        int x{};
        int y{};
        int fontSize{};
        Color color{};
    };

    struct DrawPolygon
    {
        std::vector<SDL_FPoint> vertices;
        Color color{};
    };

    using DrawCommand = std::variant<DrawRectangle, DrawCircle, DrawText, DrawPolygon>;
    using DrawList = std::vector<DrawCommand>;
}