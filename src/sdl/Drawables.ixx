module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <vector>

export module awen.sdl.drawables;

export import awen.sdl.color;

export namespace awen::sdl
{
    /// @brief Tag added to any entity that owns at least one drawable shape component.
    /// @note Currently advisory; queries use the shape components directly.
    struct Renderable
    {
    };

    /// @brief Filled axis-aligned rectangle in local space, anchored at the entity's transform.
    struct RectangleFill
    {
        glm::vec2 size{};
        glm::vec2 anchor{0.5F, 0.5F};
        Color color{colors::White};
    };

    /// @brief Outlined axis-aligned rectangle in local space.
    struct RectangleOutline
    {
        glm::vec2 size{};
        glm::vec2 anchor{0.5F, 0.5F};
        Color color{colors::White};
        float thickness{1.0F};
    };

    /// @brief Filled circle centered at the entity's transform.
    struct CircleFill
    {
        float radius{};
        Color color{colors::White};
    };

    /// @brief Outlined circle centered at the entity's transform.
    struct CircleOutline
    {
        float radius{};
        Color color{colors::White};
        float thickness{1.0F};
    };

    /// @brief Line segment in local space relative to the entity's transform.
    struct Line
    {
        glm::vec2 from{};
        glm::vec2 to{};
        Color color{colors::White};
        float thickness{1.0F};
    };

    /// @brief Polygon in local space relative to the entity's transform.
    /// @note Filled polygons are rasterized as a fan from `points[0]`.
    struct Polygon
    {
        std::vector<glm::vec2> points;
        Color color{colors::White};
        bool filled{true};
    };

    /// @brief Text label rendered with a Font resource entity.
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
}
