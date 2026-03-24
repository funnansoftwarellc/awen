module;

#include <raylib.h>

export module awen.graphics.renderer;

export import awen.graphics.color;

namespace
{
    auto to_raylib(const awn::graphics::Color& c) -> ::Color
    {
        return {c.r, c.g, c.b, c.a};
    }
}

export namespace awn::graphics
{
    class Renderer
    {
    public:
        static auto begin() -> void
        {
            BeginDrawing();
        }

        static auto end() -> void
        {
            EndDrawing();
        }

        static auto clear(const Color& color) -> void
        {
            ClearBackground(to_raylib(color));
        }

        static auto draw_rect(float x, float y, float width, float height, const Color& color) -> void
        {
            DrawRectangleV(::Vector2{.x = x, .y = y}, ::Vector2{.x = width, .y = height}, to_raylib(color));
        }

        static auto draw_circle(float center_x, float center_y, float radius, const Color& color) -> void
        {
            DrawCircleV(::Vector2{.x = center_x, .y = center_y}, radius, to_raylib(color));
        }

        static auto draw_text(const char* text, int x, int y, int font_size, const Color& color) -> void
        {
            DrawText(text, x, y, font_size, to_raylib(color));
        }

        [[nodiscard]] static auto measure_text(const char* text, int font_size) -> int
        {
            return MeasureText(text, font_size);
        }

        static auto draw_line(float start_x, float start_y, float end_x, float end_y, const Color& color) -> void
        {
            DrawLineV(::Vector2{.x = start_x, .y = start_y}, ::Vector2{.x = end_x, .y = end_y}, to_raylib(color));
        }
    };
}
