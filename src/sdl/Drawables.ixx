module;

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

    /// @brief Text label rendered with a Font resource entity.
    /// @note The renderer rasterises the text through `TTF_RenderText_Blended`
    ///       and creates a fresh `SDL_Texture` every frame. Use sparingly; a
    ///       cached-texture path is a planned follow-up.
    struct TextLabel
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
        std::variant<Rectangle, Circle, Line, Polygon, TextLabel, Sprite> shape;
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
}
