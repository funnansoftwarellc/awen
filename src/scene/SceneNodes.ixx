module;

#include <string>

export module awen.scene.scene_nodes;

export import awen.graphics.color;
export import awen.scene.texture_id;

export namespace awn::scene
{
    // Bring Color into scope so node definitions can reference it without qualification.
    using awn::graphics::Color;

    /// @brief Visual data for a filled axis-aligned rectangle node.
    struct RectNode
    {
        float width{};
        float height{};
        Color color{};
    };

    /// @brief Visual data for a textured sprite node.
    struct SpriteNode
    {
        TextureId texture_id{};
        float width{};
        float height{};
        Color tint{};
    };

    /// @brief Visual data for a text label node.
    struct TextNode
    {
        std::string text;
        int font_size{};
        Color color{};
    };
}
