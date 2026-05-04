module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <awen/flecs/Flecs.hpp>

#include <glm/vec2.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>
#include <string>
#include <variant>
#include <vector>

export module awen.sdl.renderer;

export import awen.sdl.color;
export import awen.sdl.drawables;
export import awen.sdl.resources;
export import awen.sdl.transform;

import awen.core.overloaded;

namespace awen::sdl::detail
{
    auto setColor(SDL_Renderer* sdlRenderer, Color color) -> void
    {
        SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    }

    auto toFColor(Color color) -> SDL_FColor
    {
        return SDL_FColor{
            .r = static_cast<float>(color.r) / 255.0F,
            .g = static_cast<float>(color.g) / 255.0F,
            .b = static_cast<float>(color.b) / 255.0F,
            .a = static_cast<float>(color.a) / 255.0F,
        };
    }

    auto fontFor(const flecs::world& world, flecs::entity_t fontId) -> TTF_Font*
    {
        if (fontId == 0)
        {
            return nullptr;
        }

        const auto entity = world.entity(fontId);

        if (!entity.is_valid())
        {
            return nullptr;
        }

        const auto* font = entity.try_get<Font>();

        return font != nullptr ? font->handle : nullptr;
    }

    auto textureFor(const flecs::world& world, flecs::entity_t textureId) -> const Texture*
    {
        if (textureId == 0)
        {
            return nullptr;
        }

        const auto entity = world.entity(textureId);

        if (!entity.is_valid())
        {
            return nullptr;
        }

        return entity.try_get<Texture>();
    }

    auto worldPolygon(const WorldTransform& world, const std::vector<glm::vec2>& points) -> std::vector<SDL_FPoint>
    {
        auto out = std::vector<SDL_FPoint>{};
        out.reserve(points.size());

        for (const auto& point : points)
        {
            const auto worldPoint = applyWorld(world, point);
            out.push_back(SDL_FPoint{.x = worldPoint.x, .y = worldPoint.y});
        }

        return out;
    }

    /// @brief Transform a fixed-size array of local-space points to SDL_FPoints.
    template <std::size_t N>
    auto worldPolygon(const WorldTransform& world, const std::array<glm::vec2, N>& points) -> std::array<SDL_FPoint, N>
    {
        auto out = std::array<SDL_FPoint, N>{};

        for (auto i = std::size_t{0}; i < N; ++i)
        {
            const auto worldPoint = applyWorld(world, points[i]);
            out[i] = SDL_FPoint{.x = worldPoint.x, .y = worldPoint.y};
        }

        return out;
    }

    auto drawCircleFill(SDL_Renderer* sdlRenderer, glm::vec2 center, float radius) -> void
    {
        const auto radiusInt = static_cast<int>(std::ceil(radius));

        for (auto dy = -radiusInt; dy <= radiusInt; ++dy)
        {
            const auto y = static_cast<float>(dy);
            const auto x = std::sqrt(std::max(0.0F, (radius * radius) - (y * y)));

            SDL_RenderLine(sdlRenderer, center.x - x, center.y + y, center.x + x, center.y + y);
        }
    }

    auto drawCircleOutline(SDL_Renderer* sdlRenderer, glm::vec2 center, float radius) -> void
    {
        const auto steps = std::max(16, static_cast<int>(radius * 4.0F));
        auto previous = glm::vec2{center.x + radius, center.y};

        for (auto i = 1; i <= steps; ++i)
        {
            const auto theta = (static_cast<float>(i) / static_cast<float>(steps)) * 2.0F * std::numbers::pi_v<float>;
            const auto next = glm::vec2{center.x + (radius * std::cos(theta)), center.y + (radius * std::sin(theta))};

            SDL_RenderLine(sdlRenderer, previous.x, previous.y, next.x, next.y);

            previous = next;
        }
    }

    auto drawPolygonFill(SDL_Renderer* sdlRenderer, std::span<const SDL_FPoint> points, Color color) -> void
    {
        if (points.size() < 3)
        {
            return;
        }

        const auto fillColor = toFColor(color);
        auto vertices = std::vector<SDL_Vertex>{};
        vertices.reserve(points.size());

        for (const auto& point : points)
        {
            vertices.push_back(SDL_Vertex{.position = point, .color = fillColor, .tex_coord = SDL_FPoint{}});
        }

        auto indices = std::vector<int>{};
        indices.reserve((points.size() - 2) * 3);

        for (auto i = std::size_t{1}; i + 1 < points.size(); ++i)
        {
            indices.push_back(0);
            indices.push_back(static_cast<int>(i));
            indices.push_back(static_cast<int>(i + 1));
        }

        SDL_RenderGeometry(sdlRenderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), indices.data(),
                           static_cast<int>(indices.size()));
    }

    auto drawPolygonOutline(SDL_Renderer* sdlRenderer, std::span<const SDL_FPoint> points) -> void
    {
        if (points.size() < 2)
        {
            return;
        }

        for (auto i = std::size_t{0}; i < points.size(); ++i)
        {
            const auto& a = points[i];
            const auto& b = points[(i + 1) % points.size()];

            SDL_RenderLine(sdlRenderer, a.x, a.y, b.x, b.y);
        }
    }

    auto rectangleCornerPoints(const Rectangle& rectangle) -> std::array<glm::vec2, 4>
    {
        const auto offset = rectangle.size * rectangle.anchor;
        const auto topLeft = -offset;
        const auto bottomRight = rectangle.size - offset;

        return std::array<glm::vec2, 4>{
            topLeft,
            glm::vec2{bottomRight.x, topLeft.y},
            bottomRight,
            glm::vec2{topLeft.x, bottomRight.y},
        };
    }
}

export namespace awen::sdl
{
    /// @brief Draw the entity's Drawable through SDL_Renderer using its
    ///        WorldTransform. Applies the optional Outline modifier on top.
    auto drawEntity(const flecs::world& world, SDL_Renderer* sdlRenderer, flecs::entity entity, const Drawable& drawable,
                    const WorldTransform& transform) -> void
    {
        const auto* outline = entity.try_get<Outline>();

        std::visit(
            awen::core::Overloaded{
                [&](const Rectangle& rectangle)
                {
                    const auto worldPoints = detail::worldPolygon(transform, detail::rectangleCornerPoints(rectangle));

                    detail::drawPolygonFill(sdlRenderer, worldPoints, rectangle.color);

                    if (outline != nullptr)
                    {
                        detail::setColor(sdlRenderer, outline->color);
                        detail::drawPolygonOutline(sdlRenderer, worldPoints);
                    }
                },
                [&](const Circle& circle)
                {
                    const auto center = applyWorld(transform, glm::vec2{});
                    const auto radius = circle.radius * worldScaleX(transform);

                    detail::setColor(sdlRenderer, circle.color);
                    detail::drawCircleFill(sdlRenderer, center, radius);

                    if (outline != nullptr)
                    {
                        detail::setColor(sdlRenderer, outline->color);
                        detail::drawCircleOutline(sdlRenderer, center, radius);
                    }
                },
                [&](const Line& line)
                {
                    const auto from = applyWorld(transform, line.from);
                    const auto to = applyWorld(transform, line.to);

                    detail::setColor(sdlRenderer, line.color);
                    SDL_RenderLine(sdlRenderer, from.x, from.y, to.x, to.y);
                },
                [&](const Polygon& polygon)
                {
                    const auto worldPoints = detail::worldPolygon(transform, polygon.points);

                    detail::drawPolygonFill(sdlRenderer, worldPoints, polygon.color);

                    if (outline != nullptr)
                    {
                        detail::setColor(sdlRenderer, outline->color);
                        detail::drawPolygonOutline(sdlRenderer, worldPoints);
                    }
                },
                [&](const TextLabel& label)
                {
                    auto* font = detail::fontFor(world, label.font);

                    if (font == nullptr || label.text.empty())
                    {
                        return;
                    }

                    const auto sdlColor = SDL_Color{.r = label.color.r, .g = label.color.g, .b = label.color.b, .a = label.color.a};
                    auto* surface = TTF_RenderText_Blended(font, label.text.c_str(), 0, sdlColor);

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

                    const auto position = applyWorld(transform, glm::vec2{});
                    const auto dest = SDL_FRect{
                        .x = position.x - (label.anchor.x * width),
                        .y = position.y - (label.anchor.y * height),
                        .w = width,
                        .h = height,
                    };

                    SDL_RenderTexture(sdlRenderer, texture, nullptr, &dest);
                    SDL_DestroyTexture(texture);
                },
                [&](const Sprite& sprite)
                {
                    const auto* texture = detail::textureFor(world, sprite.texture);

                    if (texture == nullptr || texture->handle == nullptr)
                    {
                        return;
                    }

                    SDL_SetTextureColorMod(texture->handle, sprite.tint.r, sprite.tint.g, sprite.tint.b);
                    SDL_SetTextureAlphaMod(texture->handle, sprite.tint.a);

                    const auto offset = sprite.size * sprite.anchor;
                    const auto topLeft = applyWorld(transform, -offset);
                    const auto scale = worldScaleX(transform);
                    const auto rotationDegrees = worldRotation(transform) * (180.0F / std::numbers::pi_v<float>);
                    const auto dest = SDL_FRect{
                        .x = topLeft.x,
                        .y = topLeft.y,
                        .w = sprite.size.x * scale,
                        .h = sprite.size.y * scale,
                    };

                    if (std::abs(rotationDegrees) < 0.001F)
                    {
                        SDL_RenderTexture(sdlRenderer, texture->handle, nullptr, &dest);
                    }
                    else
                    {
                        const auto pivot = SDL_FPoint{.x = dest.w * sprite.anchor.x, .y = dest.h * sprite.anchor.y};
                        SDL_RenderTextureRotated(sdlRenderer, texture->handle, nullptr, &dest, rotationDegrees, &pivot, SDL_FLIP_NONE);
                    }
                },
            },
            drawable.shape);
    }
}
