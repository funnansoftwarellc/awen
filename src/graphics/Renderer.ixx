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
    auto toRaylib(const awen::graphics::Color& c) -> ::Color
    {
        return {c.r, c.g, c.b, c.a};
    }
}

export namespace awen::graphics
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
            ClearBackground(toRaylib(color));
        }

        static auto drawRect(float x, float y, float width, float height, const Color& color) -> void
        {
            DrawRectangleV(::Vector2{.x = x, .y = y}, ::Vector2{.x = width, .y = height}, toRaylib(color));
        }

        static auto drawCircle(float centerX, float centerY, float radius, const Color& color) -> void
        {
            DrawCircleV(::Vector2{.x = centerX, .y = centerY}, radius, toRaylib(color));
        }

        static auto drawText(const char* text, int x, int y, int fontSize, const Color& color) -> void
        {
            ::DrawText(text, x, y, fontSize, toRaylib(color));
        }

        [[nodiscard]] static auto measureText(const char* text, int fontSize) -> int
        {
            return MeasureText(text, fontSize);
        }

        static auto drawLine(float startX, float startY, float endX, float endY, const Color& color) -> void
        {
            DrawLineV(::Vector2{.x = startX, .y = startY}, ::Vector2{.x = endX, .y = endY}, toRaylib(color));
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
                    awen::core::Overloaded{
                        [](const DrawClear& c) { ClearBackground(toRaylib(c.color)); },
                        [](const DrawRect& c)
                        { DrawRectangleV(::Vector2{.x = c.x, .y = c.y}, ::Vector2{.x = c.width, .y = c.height}, toRaylib(c.color)); },
                        [](const DrawCircle& c) { DrawCircleV(::Vector2{.x = c.centerX, .y = c.centerY}, c.radius, toRaylib(c.color)); },
                        [](const DrawLine& c)
                        { DrawLineV(::Vector2{.x = c.startX, .y = c.startY}, ::Vector2{.x = c.endX, .y = c.endY}, toRaylib(c.color)); },
                        [](const DrawText& c) { ::DrawText(c.text.c_str(), c.x, c.y, c.fontSize, toRaylib(c.color)); },
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
                                toRaylib(c.tint));
                        },
                    },
                    cmd);
            }
        }
    };
}
