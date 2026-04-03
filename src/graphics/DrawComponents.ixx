module;

#include <string>

export module awen.graphics.draw_components;

export import awen.graphics.color;
export import awen.graphics.texture_id;

export namespace awn::graphics
{
    /// @brief ECS visual component for a filled axis-aligned rectangle.
    ///
    /// Attach to a flecs entity alongside a Transform to draw a rectangle.
    /// Position is taken from the entity's WorldTransform during the render pass.
    struct DrawRect
    {
        float width{};
        float height{};
        Color color{};
    };

    /// @brief ECS visual component for a filled circle.
    ///
    /// Attach to a flecs entity alongside a Transform to draw a circle.
    /// Position is taken from the entity's WorldTransform during the render pass.
    struct DrawCircle
    {
        float radius{};
        Color color{};
    };

    /// @brief ECS visual component for a text label.
    ///
    /// Attach to a flecs entity alongside a Transform to draw text.
    /// Position is taken from the entity's WorldTransform during the render pass.
    struct DrawText
    {
        std::string text;
        int font_size{};
        Color color{};
    };

    /// @brief ECS visual component for a textured sprite.
    ///
    /// Attach to a flecs entity alongside a Transform to draw a sprite.
    /// Position is taken from the entity's WorldTransform during the render pass.
    struct DrawSprite
    {
        TextureId texture_id{};
        float width{};
        float height{};
        Color tint{};
    };
}
