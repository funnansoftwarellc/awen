module;

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

export module awen.graphics.draw_list;

export import awen.graphics.color;

export namespace awn::graphics
{
    /// @brief Render command that clears the background to a solid colour.
    struct RenderClear
    {
        Color color{};
    };

    /// @brief Render command that draws a filled axis-aligned rectangle at a world position.
    struct RenderRect
    {
        float x{};
        float y{};
        float width{};
        float height{};
        Color color{};
    };

    /// @brief Render command that draws a filled circle at a world position.
    struct RenderCircle
    {
        float x{};
        float y{};
        float radius{};
        Color color{};
    };

    /// @brief Render command that draws a line segment between two world-space points.
    struct RenderLine
    {
        float start_x{};
        float start_y{};
        float end_x{};
        float end_y{};
        Color color{};
    };

    /// @brief Render command that draws a string of text at a world position using the default font.
    struct RenderText
    {
        std::string text;
        int x{};
        int y{};
        int font_size{};
        Color color{};
    };

    /// @brief Render command that begins a scissor (clipping) rectangle.
    ///
    /// All subsequent render commands until a matching RenderEndScissor are clipped
    /// to the region defined by this command.
    struct RenderBeginScissor
    {
        int x{};
        int y{};
        int width{};
        int height{};
    };

    /// @brief Render command that ends the current scissor region.
    struct RenderEndScissor
    {
    };

    /// @brief GPU texture descriptor embedded in a DrawSprite command.
    ///
    /// Contains only the fields the renderer needs to issue a draw call.
    /// Populated by the scene traversal pass from a TextureCache lookup so that
    /// the DrawList itself has no dependency on the TextureCache.
    struct TextureHandle
    {
        unsigned int id{};
        int width{};
        int height{};
        int mipmaps{};
        int format{};
    };

    /// @brief Render command that draws a textured rectangle at a world position.
    struct RenderSprite
    {
        TextureHandle texture{};
        float x{};
        float y{};
        float width{};
        float height{};
        Color tint{};
    };

    /// @brief A single entry in a DrawList, holding one draw command of any supported type.
    using DrawCommand =
        std::variant<RenderClear, RenderRect, RenderCircle, RenderLine, RenderText, RenderBeginScissor, RenderEndScissor, RenderSprite>;

    /// @brief An ordered list of draw commands consumed by Renderer::submit().
    ///
    /// Built each frame during the scene traversal pass and then handed to the
    /// renderer. The list owns no GPU resources — all values are plain data.
    class DrawList
    {
    public:
        /// @brief Removes all commands from the list without releasing backing memory.
        auto clear() -> void
        {
            commands_.clear();
        }

        /// @brief Appends a draw command to the end of the list.
        /// @param cmd The command to append.
        auto push(DrawCommand cmd) -> void
        {
            commands_.emplace_back(std::move(cmd));
        }

        /// @brief Returns a read-only view of the command list.
        [[nodiscard]] auto commands() const noexcept -> const std::vector<DrawCommand>&
        {
            return commands_;
        }

        /// @brief Returns the number of commands currently in the list.
        [[nodiscard]] auto size() const noexcept -> std::size_t
        {
            return std::size(commands_);
        }

        /// @brief Returns true when the list contains no commands.
        [[nodiscard]] auto empty() const noexcept -> bool
        {
            return std::empty(commands_);
        }

    private:
        std::vector<DrawCommand> commands_;
    };
}
