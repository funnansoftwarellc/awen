module;

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <SDL3/SDL_render.h>

export module awen.sdl.drawlist;

import awen.sdl.color;
import awen.sdl.font;

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
        std::optional<FontHandle> font;
    };

    struct DrawPolygon
    {
        std::vector<SDL_FPoint> vertices;
        Color color{};
    };

    /// @brief Pushes a clipping rectangle onto the renderer's scissor stack.
    struct DrawScissorPush
    {
        int x{};
        int y{};
        int width{};
        int height{};
    };

    /// @brief Pops the most recent clipping rectangle off the scissor stack.
    struct DrawScissorPop
    {
    };

    using DrawCommand = std::variant<DrawRectangle, DrawCircle, DrawText, DrawPolygon, DrawScissorPush, DrawScissorPop>;
    using DrawList = std::vector<DrawCommand>;
}
