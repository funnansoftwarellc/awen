module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

export module awen.sdl.renderer;

import awen.core.overloaded;
import awen.sdl.color;
import awen.sdl.drawlist;

namespace
{
    auto MakeSdlError(std::string_view message) -> std::string
    {
        return std::string{message} + ": " + SDL_GetError();
    }

    auto ToSdlColor(const awen::sdl::Color& color) -> SDL_Color
    {
        return SDL_Color{.r = color.r, .g = color.g, .b = color.b, .a = color.a};
    }

    auto SetDrawColor(SDL_Renderer* renderer, const awen::sdl::Color& color) -> std::expected<void, std::string>
    {
        if (!SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a))
        {
            return std::unexpected(MakeSdlError("Failed to set SDL renderer draw color"));
        }

        return {};
    }

    auto FontPaths() -> const std::vector<std::string>&
    {
        static const auto paths = []
        {
            auto resolvedPaths = std::vector<std::string>{
#if defined(__EMSCRIPTEN__)
                "/fonts/DejaVuSans.ttf",
#elif defined(_WIN32)
                "C:/Windows/Fonts/segoeui.ttf",
                "C:/Windows/Fonts/arial.ttf",
                "C:/Windows/Fonts/tahoma.ttf",
#elif defined(__ANDROID__)
                "/system/fonts/Roboto-Regular.ttf",
                "/system/fonts/DroidSans.ttf",
                "/system/fonts/NotoSans-Regular.ttf",
#else
                "/System/Library/Fonts/Geneva.ttf",
                "/System/Library/Fonts/Monaco.ttf",
                "/System/Library/Fonts/Helvetica.ttc",
                "/Library/Fonts/Arial Unicode.ttf",
                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
#endif
            };

            if (auto* basePathRaw = SDL_GetBasePath(); basePathRaw != nullptr)
            {
                auto basePath = std::string{basePathRaw};

                resolvedPaths.emplace(resolvedPaths.begin(), basePath + "fonts/DejaVuSans.ttf");
            }

            resolvedPaths.emplace(resolvedPaths.begin(), "fonts/DejaVuSans.ttf");
            resolvedPaths.emplace(resolvedPaths.begin(), "assets/fonts/DejaVuSans.ttf");
            resolvedPaths.emplace(resolvedPaths.begin(), "../assets/fonts/DejaVuSans.ttf");
            resolvedPaths.emplace(resolvedPaths.begin(), "../../assets/fonts/DejaVuSans.ttf");

            return resolvedPaths;
        }();

        return paths;
    }
}

export namespace awen::sdl
{
    class Renderer
    {
    public:
        static auto initialize(SDL_Renderer* renderer) -> std::expected<void, std::string>
        {
            if (renderer == nullptr)
            {
                return std::unexpected(std::string{"Cannot initialize SDL renderer with a null handle"});
            }

            renderer_ = renderer;
            return {};
        }

        static auto shutdown() -> void
        {
            fonts_.clear();
            renderer_ = nullptr;
        }

        static auto begin() -> std::expected<void, std::string>
        {
            if (renderer_ == nullptr)
            {
                return std::unexpected(std::string{"SDL renderer has not been initialized"});
            }

            return {};
        }

        static auto clear(Color color) -> std::expected<void, std::string>
        {
            if (auto result = SetDrawColor(renderer_, color); !result)
            {
                return result;
            }

            if (!SDL_RenderClear(renderer_))
            {
                return std::unexpected(MakeSdlError("Failed to clear SDL renderer"));
            }

            return {};
        }

        static auto submit(const DrawList& drawList) -> std::expected<void, std::string>
        {
            for (const auto& command : drawList)
            {
                auto result = std::visit(
                    awen::core::Overloaded{
                        [](const DrawRectangle& rectangle) -> std::expected<void, std::string>
                        {
                            if (auto colorResult = SetDrawColor(renderer_, rectangle.color); !colorResult)
                            {
                                return colorResult;
                            }

                            const auto rect = SDL_FRect{.x = rectangle.x, .y = rectangle.y, .w = rectangle.width, .h = rectangle.height};

                            if (!SDL_RenderFillRect(renderer_, &rect))
                            {
                                return std::unexpected(MakeSdlError("Failed to draw SDL rectangle"));
                            }

                            return {};
                        },
                        [](const DrawCircle& circle) -> std::expected<void, std::string>
                        {
                            if (auto colorResult = SetDrawColor(renderer_, circle.color); !colorResult)
                            {
                                return colorResult;
                            }

                            const auto radiusInt = static_cast<int>(std::ceil(circle.radius));

                            for (auto dy = -radiusInt; dy <= radiusInt; ++dy)
                            {
                                const auto y = static_cast<float>(dy);
                                const auto x = std::sqrt(std::max(0.0F, (circle.radius * circle.radius) - (y * y)));

                                if (!SDL_RenderLine(renderer_, circle.centerX - x, circle.centerY + y, circle.centerX + x, circle.centerY + y))
                                {
                                    return std::unexpected(MakeSdlError("Failed to draw SDL circle"));
                                }
                            }

                            return {};
                        },
                        [](const DrawText& text) -> std::expected<void, std::string>
                        {
                            auto fontResult = ensureFont(text.fontSize);

                            if (!fontResult)
                            {
                                return std::unexpected(fontResult.error());
                            }

                            auto* surface = TTF_RenderText_Blended(*fontResult, text.text.c_str(), 0, ToSdlColor(text.color));

                            if (surface == nullptr)
                            {
                                return std::unexpected(MakeSdlError("Failed to render SDL_ttf text"));
                            }

                            auto* texture = SDL_CreateTextureFromSurface(renderer_, surface);

                            if (texture == nullptr)
                            {
                                SDL_DestroySurface(surface);
                                return std::unexpected(MakeSdlError("Failed to create SDL text texture"));
                            }

                            const auto dest = SDL_FRect{
                                .x = static_cast<float>(text.x),
                                .y = static_cast<float>(text.y),
                                .w = static_cast<float>(surface->w),
                                .h = static_cast<float>(surface->h),
                            };

                            SDL_DestroySurface(surface);

                            if (!SDL_RenderTexture(renderer_, texture, nullptr, &dest))
                            {
                                SDL_DestroyTexture(texture);
                                return std::unexpected(MakeSdlError("Failed to draw SDL text texture"));
                            }

                            SDL_DestroyTexture(texture);
                            return {};
                        },
                        [](const DrawPolygon& polygon) -> std::expected<void, std::string>
                        {
                            if (polygon.vertices.size() < 2)
                            {
                                return {};
                            }

                            if (auto colorResult = SetDrawColor(renderer_, polygon.color); !colorResult)
                            {
                                return colorResult;
                            }

                            for (auto i = std::size_t{0}; i < polygon.vertices.size(); ++i)
                            {
                                const auto& start = polygon.vertices[i];
                                const auto& end = polygon.vertices[(i + 1) % polygon.vertices.size()];

                                if (!SDL_RenderLine(renderer_, start.x, start.y, end.x, end.y))
                                {
                                    return std::unexpected(MakeSdlError("Failed to draw SDL polygon"));
                                }
                            }

                            return {};
                        },
                    },
                    command);

                if (!result)
                {
                    return result;
                }
            }

            return {};
        }

        static auto end() -> std::expected<void, std::string>
        {
            if (!SDL_RenderPresent(renderer_))
            {
                return std::unexpected(MakeSdlError("Failed to present SDL frame"));
            }

            return {};
        }

        [[nodiscard]] static auto measureText(const char* text, int fontSize) -> std::expected<int, std::string>
        {
            auto fontResult = ensureFont(fontSize);

            if (!fontResult)
            {
                return std::unexpected(fontResult.error());
            }

            auto width = 0;
            auto height = 0;

            if (!TTF_GetStringSize(*fontResult, text, 0, &width, &height))
            {
                return std::unexpected(MakeSdlError("Failed to measure SDL_ttf text"));
            }

            return width;
        }

    private:
        static auto ensureFont(int fontSize) -> std::expected<TTF_Font*, std::string>
        {
            if (fontSize <= 0)
            {
                return std::unexpected(std::string{"Font size must be positive"});
            }

            if (const auto it = fonts_.find(fontSize); it != std::end(fonts_))
            {
                return it->second.get();
            }

            for (const auto& path : FontPaths())
            {
                if (auto* rawFont = TTF_OpenFont(path.c_str(), static_cast<float>(fontSize)); rawFont != nullptr)
                {
                    auto deleter = [](TTF_Font* font) { TTF_CloseFont(font); };

                    auto [it, inserted] = fonts_.emplace(fontSize, std::unique_ptr<TTF_Font, decltype(deleter)>{rawFont, deleter});
                    std::ignore = inserted;
                    return it->second.get();
                }
            }

            return std::unexpected(std::string{"Failed to open an SDL_ttf font from the configured fallback paths"});
        }

        static inline SDL_Renderer* renderer_ = nullptr;                                                // NOLINT(readability-identifier-naming)
        static inline std::unordered_map<int, std::unique_ptr<TTF_Font, void (*)(TTF_Font*)>> fonts_{}; // NOLINT(readability-identifier-naming)
    };
}