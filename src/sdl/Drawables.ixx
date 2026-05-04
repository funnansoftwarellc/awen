module;

#include <SDL3/SDL.h>
#include <awen/flecs/Flecs.hpp>

#include <glm/vec2.hpp>

#include <string>
#include <variant>
#include <vector>

export module awen.sdl.drawables;

export import awen.sdl.color;

export namespace awen::sdl
{
    /// @brief Filled axis-aligned rectangle.
    struct Rectangle
    {
        glm::vec2 size{};
        glm::vec2 anchor{0.5F, 0.5F};
        Color color{colors::White};
    };

    /// @brief Filled circle centred at the entity's transform.
    struct Circle
    {
        float radius{};
        Color color{colors::White};
    };

    /// @brief Line segment in local space.
    /// @note `thickness` is currently ignored by the SDL renderer (it draws
    ///       a 1-pixel line). Reserved for a future geometry-based path.
    struct Line
    {
        glm::vec2 from{};
        glm::vec2 to{};
        Color color{colors::White};
        float thickness{1.0F};
    };

    /// @brief Polygon in local space.
    /// @note Filled polygons are rasterised as a fan from `points[0]`.
    struct Polygon
    {
        std::vector<glm::vec2> points;
        Color color{colors::White};
    };

    /// @brief Text rendered with a Font resource entity.
    /// @note The renderer caches the rasterised texture in a TextCache
    ///       component on the entity and only re-rasterises when `text`,
    ///       `font`, or `color` change.
    struct Text
    {
        std::string text;
        flecs::entity_t font{};
        glm::vec2 anchor{0.0F, 0.0F};
        Color color{colors::White};
    };

    /// @brief Sprite drawn from a Texture resource entity.
    struct Sprite
    {
        flecs::entity_t texture{};
        glm::vec2 size{};
        glm::vec2 anchor{0.5F, 0.5F};
        Color tint{colors::White};
    };

    /// @brief Drawable component. Pick exactly one shape kind per entity.
    struct Drawable
    {
        std::variant<Rectangle, Circle, Line, Polygon, Text, Sprite> shape;
    };

    /// @brief Optional outline modifier. Drawn after the entity's Drawable.
    /// @note Only meaningful when paired with a Rectangle, Circle, or Polygon.
    /// @note `thickness` is currently ignored by the SDL renderer (1-pixel
    ///       outline). Reserved for a future geometry-based path.
    struct Outline
    {
        Color color{colors::White};
        float thickness{1.0F};
    };

    /// @brief Internal cached SDL_Texture for a Text drawable.
    /// @note Attached automatically by the renderer the first time a
    ///       Text entity is drawn. Reused on subsequent frames as long
    ///       as `text`, `font`, `color`, and `renderer` are unchanged.
    ///       Cleaned up by an OnRemove observer (registered by the Module).
    struct TextCache
    {
        SDL_Texture* handle{};
        SDL_Renderer* renderer{};
        int width{};
        int height{};
        std::string text;
        flecs::entity_t font{};
        Color color{};
    };
}
