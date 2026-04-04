module;

#include <algorithm>
#include <iterator>
#include <variant>

#include <raylib.h>

export module awen.graphics.renderer;

import awen.core.overloaded;

export import awen.graphics.color;
export import awen.graphics.draw_list;

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
            ::DrawText(text, x, y, font_size, to_raylib(color));
        }

        [[nodiscard]] static auto measure_text(const char* text, int font_size) -> int
        {
            return MeasureText(text, font_size);
        }

        static auto draw_line(float start_x, float start_y, float end_x, float end_y, const Color& color) -> void
        {
            DrawLineV(::Vector2{.x = start_x, .y = start_y}, ::Vector2{.x = end_x, .y = end_y}, to_raylib(color));
        }

        /// @brief Submits an ordered list of draw commands to the underlying renderer.
        ///
        /// Each command in @p list is dispatched to the corresponding raylib call.
        /// This method must be called between Renderer::begin() and Renderer::end().
        /// @param list The draw list to consume.
        static auto submit(const DrawList& list) -> void
        {
            for (const auto& cmd : list.commands())
            {
                std::visit(
                    awn::core::Overloaded{
                        [](const DrawClear& c) { ClearBackground(to_raylib(c.color)); },
                        [](const DrawRect& c)
                        { DrawRectangleV(::Vector2{.x = c.x, .y = c.y}, ::Vector2{.x = c.width, .y = c.height}, to_raylib(c.color)); },
                        [](const DrawCircle& c) { DrawCircleV(::Vector2{.x = c.center_x, .y = c.center_y}, c.radius, to_raylib(c.color)); },
                        [](const DrawLine& c)
                        { DrawLineV(::Vector2{.x = c.start_x, .y = c.start_y}, ::Vector2{.x = c.end_x, .y = c.end_y}, to_raylib(c.color)); },
                        [](const DrawText& c) { ::DrawText(c.text.c_str(), c.x, c.y, c.font_size, to_raylib(c.color)); },
                        [](const DrawBeginScissor& c) { BeginScissorMode(c.x, c.y, c.width, c.height); },
                        [](const DrawEndScissor&) { EndScissorMode(); },
                        [](const DrawSprite& c)
                        {
                            const auto tex = Texture2D{
                                .id = c.texture.id,
                                .width = c.texture.width,
                                .height = c.texture.height,
                                .mipmaps = c.texture.mipmaps,
                                .format = c.texture.format,
                            };
                            DrawTexturePro(
                                tex,
                                Rectangle{.x = 0.0F, .y = 0.0F, .width = static_cast<float>(tex.width), .height = static_cast<float>(tex.height)},
                                Rectangle{.x = c.x, .y = c.y, .width = c.width, .height = c.height}, Vector2{.x = 0.0F, .y = 0.0F}, 0.0F,
                                to_raylib(c.tint));
                        },
                    },
                    cmd);
            }
        }
    };
}
