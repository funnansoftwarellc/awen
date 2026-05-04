module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <awen/flecs/Flecs.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <glm/vec2.hpp>
#include <string>
#include <variant>
#include <vector>

export module awen.sdl.renderer;

export import awen.sdl.color;
export import awen.sdl.drawables;
export import awen.sdl.resources;
export import awen.sdl.transform;

import awen.core.overloaded;

export namespace awen::sdl
{
    /// @brief Renderer-internal: a resolved draw command with world-space placement.
    struct DrawCommand
    {
        struct Rect
        {
            float x{};
            float y{};
            float w{};
            float h{};
            Color color{};
        };

        struct RectOutline
        {
            float x{};
            float y{};
            float w{};
            float h{};
            Color color{};
            float thickness{1.0F};
        };

        struct Circle
        {
            float cx{};
            float cy{};
            float radius{};
            Color color{};
            bool filled{true};
            float thickness{1.0F};
        };

        struct LineCmd
        {
            float x1{};
            float y1{};
            float x2{};
            float y2{};
            Color color{};
            float thickness{1.0F};
        };

        struct Poly
        {
            std::vector<SDL_FPoint> points;
            Color color{};
            bool filled{true};
        };

        struct Text
        {
            std::string text;
            float x{};
            float y{};
            glm::vec2 anchor{};
            flecs::entity_t font{};
            Color color{};
        };

        struct SpriteCmd
        {
            flecs::entity_t texture{};
            float x{};
            float y{};
            float w{};
            float h{};
            float rotationDegrees{};
            Color tint{};
        };

        using Variant = std::variant<Rect, RectOutline, Circle, LineCmd, Poly, Text, SpriteCmd>;

        Variant payload;
        std::int32_t z{};
    };

    /// @brief Singleton component holding the per-frame, sorted draw command list.
    struct DrawList
    {
        std::vector<DrawCommand> commands;
    };

    namespace detail
    {
        /// @brief Transform a local-space point through a WorldTransform.
        inline auto applyWorld(const WorldTransform& w, glm::vec2 local) -> glm::vec2
        {
            const auto cosR = std::cos(w.rotation);
            const auto sinR = std::sin(w.rotation);
            const auto scaled = glm::vec2{local.x * w.scale.x, local.y * w.scale.y};

            return glm::vec2{
                w.position.x + (scaled.x * cosR) - (scaled.y * sinR),
                w.position.y + (scaled.x * sinR) + (scaled.y * cosR),
            };
        }

        inline auto worldOf(flecs::entity e) -> WorldTransform
        {
            if (const auto* w = e.try_get<WorldTransform>())
            {
                return *w;
            }

            return WorldTransform{};
        }

        inline auto pickZ(flecs::entity e) -> std::int32_t
        {
            if (const auto* z = e.try_get<ZOrder>())
            {
                return z->value;
            }

            return 0;
        }
    }

    /// @brief Build the draw list from drawable component queries.
    /// @note Run during the OnPreRender phase, after transform propagation.
    inline auto buildDrawList(flecs::world world) -> void
    {
        auto& list = world.ensure<DrawList>();
        list.commands.clear();

        world.each(
            [&list](flecs::entity e, const RectangleFill& shape)
            {
                const auto w = detail::worldOf(e);
                const auto offset = glm::vec2{shape.size.x * shape.anchor.x, shape.size.y * shape.anchor.y};
                const auto topLeft = detail::applyWorld(w, -offset);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::Rect{
                        .x = topLeft.x,
                        .y = topLeft.y,
                        .w = shape.size.x * w.scale.x,
                        .h = shape.size.y * w.scale.y,
                        .color = shape.color,
                    },
                    .z = detail::pickZ(e),
                });
            });

        world.each(
            [&list](flecs::entity e, const RectangleOutline& shape)
            {
                const auto w = detail::worldOf(e);
                const auto offset = glm::vec2{shape.size.x * shape.anchor.x, shape.size.y * shape.anchor.y};
                const auto topLeft = detail::applyWorld(w, -offset);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::RectOutline{
                        .x = topLeft.x,
                        .y = topLeft.y,
                        .w = shape.size.x * w.scale.x,
                        .h = shape.size.y * w.scale.y,
                        .color = shape.color,
                        .thickness = shape.thickness,
                    },
                    .z = detail::pickZ(e),
                });
            });

        world.each(
            [&list](flecs::entity e, const CircleFill& shape)
            {
                const auto w = detail::worldOf(e);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::Circle{
                        .cx = w.position.x,
                        .cy = w.position.y,
                        .radius = shape.radius * w.scale.x,
                        .color = shape.color,
                        .filled = true,
                    },
                    .z = detail::pickZ(e),
                });
            });

        world.each(
            [&list](flecs::entity e, const CircleOutline& shape)
            {
                const auto w = detail::worldOf(e);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::Circle{
                        .cx = w.position.x,
                        .cy = w.position.y,
                        .radius = shape.radius * w.scale.x,
                        .color = shape.color,
                        .filled = false,
                        .thickness = shape.thickness,
                    },
                    .z = detail::pickZ(e),
                });
            });

        world.each(
            [&list](flecs::entity e, const Line& shape)
            {
                const auto w = detail::worldOf(e);
                const auto p1 = detail::applyWorld(w, shape.from);
                const auto p2 = detail::applyWorld(w, shape.to);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::LineCmd{
                        .x1 = p1.x,
                        .y1 = p1.y,
                        .x2 = p2.x,
                        .y2 = p2.y,
                        .color = shape.color,
                        .thickness = shape.thickness,
                    },
                    .z = detail::pickZ(e),
                });
            });

        world.each(
            [&list](flecs::entity e, const Polygon& shape)
            {
                const auto w = detail::worldOf(e);
                auto cmd = DrawCommand::Poly{.color = shape.color, .filled = shape.filled};
                cmd.points.reserve(shape.points.size());

                for (const auto& p : shape.points)
                {
                    const auto worldPoint = detail::applyWorld(w, p);
                    cmd.points.push_back(SDL_FPoint{.x = worldPoint.x, .y = worldPoint.y});
                }

                list.commands.push_back(DrawCommand{.payload = std::move(cmd), .z = detail::pickZ(e)});
            });

        world.each(
            [&list](flecs::entity e, const TextLabel& shape)
            {
                const auto w = detail::worldOf(e);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::Text{
                        .text = shape.text,
                        .x = w.position.x,
                        .y = w.position.y,
                        .anchor = shape.anchor,
                        .font = shape.font,
                        .color = shape.color,
                    },
                    .z = detail::pickZ(e),
                });
            });

        world.each(
            [&list](flecs::entity e, const Sprite& shape)
            {
                const auto w = detail::worldOf(e);
                const auto offset = glm::vec2{shape.size.x * shape.anchor.x, shape.size.y * shape.anchor.y};
                const auto topLeft = detail::applyWorld(w, -offset);

                list.commands.push_back(DrawCommand{
                    .payload = DrawCommand::SpriteCmd{
                        .texture = shape.texture,
                        .x = topLeft.x,
                        .y = topLeft.y,
                        .w = shape.size.x * w.scale.x,
                        .h = shape.size.y * w.scale.y,
                        .rotationDegrees = w.rotation * (180.0F / 3.14159265358979F),
                        .tint = shape.tint,
                    },
                    .z = detail::pickZ(e),
                });
            });
    }

    /// @brief Stable-sort the draw list by Z order ascending.
    inline auto sortDrawList(flecs::world world) -> void
    {
        auto& list = world.ensure<DrawList>();
        std::stable_sort(list.commands.begin(), list.commands.end(), [](const DrawCommand& a, const DrawCommand& b) { return a.z < b.z; });
    }

    namespace detail
    {
        inline auto setColor(SDL_Renderer* r, Color c) -> void
        {
            SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
        }

        inline auto drawCircleOutline(SDL_Renderer* r, float cx, float cy, float radius) -> void
        {
            constexpr auto pi = 3.14159265358979F;
            const auto steps = std::max(16, static_cast<int>(radius * 4.0F));
            auto prevX = cx + radius;
            auto prevY = cy;

            for (auto i = 1; i <= steps; ++i)
            {
                const auto theta = (static_cast<float>(i) / static_cast<float>(steps)) * 2.0F * pi;
                const auto x = cx + (radius * std::cos(theta));
                const auto y = cy + (radius * std::sin(theta));

                SDL_RenderLine(r, prevX, prevY, x, y);

                prevX = x;
                prevY = y;
            }
        }

        inline auto drawCircleFill(SDL_Renderer* r, float cx, float cy, float radius) -> void
        {
            const auto radiusInt = static_cast<int>(std::ceil(radius));

            for (auto dy = -radiusInt; dy <= radiusInt; ++dy)
            {
                const auto y = static_cast<float>(dy);
                const auto x = std::sqrt(std::max(0.0F, (radius * radius) - (y * y)));

                SDL_RenderLine(r, cx - x, cy + y, cx + x, cy + y);
            }
        }

        inline auto drawPolygonFill(SDL_Renderer* r, const std::vector<SDL_FPoint>& pts, Color color) -> void
        {
            if (pts.size() < 3)
            {
                return;
            }

            auto vertices = std::vector<SDL_Vertex>{};
            vertices.reserve(pts.size());

            const auto fc = SDL_FColor{
                .r = static_cast<float>(color.r) / 255.0F,
                .g = static_cast<float>(color.g) / 255.0F,
                .b = static_cast<float>(color.b) / 255.0F,
                .a = static_cast<float>(color.a) / 255.0F,
            };

            for (const auto& p : pts)
            {
                vertices.push_back(SDL_Vertex{.position = p, .color = fc, .tex_coord = SDL_FPoint{}});
            }

            auto indices = std::vector<int>{};
            indices.reserve((pts.size() - 2) * 3);

            for (auto i = std::size_t{1}; i + 1 < pts.size(); ++i)
            {
                indices.push_back(0);
                indices.push_back(static_cast<int>(i));
                indices.push_back(static_cast<int>(i + 1));
            }

            SDL_RenderGeometry(r, nullptr, vertices.data(), static_cast<int>(vertices.size()), indices.data(), static_cast<int>(indices.size()));
        }

        inline auto drawPolygonOutline(SDL_Renderer* r, const std::vector<SDL_FPoint>& pts) -> void
        {
            if (pts.size() < 2)
            {
                return;
            }

            for (auto i = std::size_t{0}; i < pts.size(); ++i)
            {
                const auto& a = pts[i];
                const auto& b = pts[(i + 1) % pts.size()];

                SDL_RenderLine(r, a.x, a.y, b.x, b.y);
            }
        }

        inline auto fontFor(flecs::world world, flecs::entity_t fontId) -> TTF_Font*
        {
            if (fontId == 0)
            {
                return nullptr;
            }

            auto entity = world.entity(fontId);

            if (!entity.is_valid())
            {
                return nullptr;
            }

            const auto* font = entity.try_get<Font>();
            return font != nullptr ? font->handle : nullptr;
        }

        inline auto textureFor(flecs::world world, flecs::entity_t texId) -> const Texture*
        {
            if (texId == 0)
            {
                return nullptr;
            }

            auto entity = world.entity(texId);

            if (!entity.is_valid())
            {
                return nullptr;
            }

            return entity.try_get<Texture>();
        }
    }

    /// @brief Dispatch the sorted draw list to the SDL renderer.
    inline auto dispatchDrawList(flecs::world world, SDL_Renderer* sdlRenderer) -> void
    {
        const auto& list = world.ensure<DrawList>();

        for (const auto& cmd : list.commands)
        {
            std::visit(
                awen::core::Overloaded{
                    [sdlRenderer](const DrawCommand::Rect& x)
                    {
                        detail::setColor(sdlRenderer, x.color);
                        const auto rect = SDL_FRect{.x = x.x, .y = x.y, .w = x.w, .h = x.h};
                        SDL_RenderFillRect(sdlRenderer, &rect);
                    },
                    [sdlRenderer](const DrawCommand::RectOutline& x)
                    {
                        detail::setColor(sdlRenderer, x.color);
                        const auto rect = SDL_FRect{.x = x.x, .y = x.y, .w = x.w, .h = x.h};
                        SDL_RenderRect(sdlRenderer, &rect);
                    },
                    [sdlRenderer](const DrawCommand::Circle& x)
                    {
                        detail::setColor(sdlRenderer, x.color);

                        if (x.filled)
                        {
                            detail::drawCircleFill(sdlRenderer, x.cx, x.cy, x.radius);
                        }
                        else
                        {
                            detail::drawCircleOutline(sdlRenderer, x.cx, x.cy, x.radius);
                        }
                    },
                    [sdlRenderer](const DrawCommand::LineCmd& x)
                    {
                        detail::setColor(sdlRenderer, x.color);
                        SDL_RenderLine(sdlRenderer, x.x1, x.y1, x.x2, x.y2);
                    },
                    [sdlRenderer](const DrawCommand::Poly& x)
                    {
                        if (x.filled)
                        {
                            detail::drawPolygonFill(sdlRenderer, x.points, x.color);
                        }
                        else
                        {
                            detail::setColor(sdlRenderer, x.color);
                            detail::drawPolygonOutline(sdlRenderer, x.points);
                        }
                    },
                    [sdlRenderer, &world](const DrawCommand::Text& x)
                    {
                        auto* font = detail::fontFor(world, x.font);

                        if (font == nullptr || x.text.empty())
                        {
                            return;
                        }

                        const auto sdlColor = SDL_Color{.r = x.color.r, .g = x.color.g, .b = x.color.b, .a = x.color.a};
                        auto* surface = TTF_RenderText_Blended(font, x.text.c_str(), 0, sdlColor);

                        if (surface == nullptr)
                        {
                            return;
                        }

                        auto* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);

                        if (texture == nullptr)
                        {
                            SDL_DestroySurface(surface);
                            return;
                        }

                        const auto width = static_cast<float>(surface->w);
                        const auto height = static_cast<float>(surface->h);
                        SDL_DestroySurface(surface);

                        const auto dest = SDL_FRect{
                            .x = x.x - (x.anchor.x * width),
                            .y = x.y - (x.anchor.y * height),
                            .w = width,
                            .h = height,
                        };

                        SDL_RenderTexture(sdlRenderer, texture, nullptr, &dest);
                        SDL_DestroyTexture(texture);
                    },
                    [sdlRenderer, &world](const DrawCommand::SpriteCmd& x)
                    {
                        const auto* tex = detail::textureFor(world, x.texture);

                        if (tex == nullptr || tex->handle == nullptr)
                        {
                            return;
                        }

                        SDL_SetTextureColorMod(tex->handle, x.tint.r, x.tint.g, x.tint.b);
                        SDL_SetTextureAlphaMod(tex->handle, x.tint.a);

                        const auto dest = SDL_FRect{.x = x.x, .y = x.y, .w = x.w, .h = x.h};

                        if (std::abs(x.rotationDegrees) < 0.001F)
                        {
                            SDL_RenderTexture(sdlRenderer, tex->handle, nullptr, &dest);
                        }
                        else
                        {
                            const auto center = SDL_FPoint{.x = x.w * 0.5F, .y = x.h * 0.5F};
                            SDL_RenderTextureRotated(sdlRenderer, tex->handle, nullptr, &dest, x.rotationDegrees, &center, SDL_FLIP_NONE);
                        }
                    },
                },
                cmd.payload);
        }
    }
}
