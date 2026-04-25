module;

#include <awen/flecs/Flecs.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

export module awen.widget.nodetext;

import awen.core.engine;
import awen.sdl.font;
import awen.widget.color;
import awen.widget.components;
import awen.widget.node;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Drawable text node synchronized into flecs each frame.
    class NodeText : public Node
    {
    public:
        /// @brief Sets the text string.
        /// @param text The UTF-8 text to render.
        auto setText(std::string_view text) -> void
        {
            text_ = text;
        }

        /// @brief Returns the current text string.
        /// @return The text value.
        [[nodiscard]] auto getText() const -> std::string_view
        {
            return text_;
        }

        /// @brief Sets the font size.
        /// @param fontSize The font size in points.
        auto setFontSize(int fontSize) -> void
        {
            fontSize_ = fontSize;
        }

        /// @brief Returns the font size.
        /// @return The font size in points.
        [[nodiscard]] auto getFontSize() const -> int
        {
            return fontSize_;
        }

        /// @brief Sets the text color.
        /// @param color The text color.
        auto setColor(Color color) -> void
        {
            color_ = color;
        }

        /// @brief Returns the current text color.
        /// @return The text color.
        [[nodiscard]] auto getColor() const -> Color
        {
            return color_;
        }

        /// @brief Selects a specific font for this text node.
        /// @param font The font handle (path + size).  Pass `std::nullopt` to use the default font.
        auto setFont(std::optional<awen::sdl::FontHandle> font) -> void
        {
            font_ = std::move(font);
        }

        /// @brief Returns the currently selected font handle, if any.
        /// @return The font handle or empty optional.
        [[nodiscard]] auto getFont() const -> const std::optional<awen::sdl::FontHandle>&
        {
            return font_;
        }

        /// @brief Synchronizes text state into the associated flecs entity.
        /// @param entity The entity to update.
        /// @return The updated entity.
        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Drawable>(components::Drawable{
                .value = std::make_shared<components::DrawableVariant>(std::in_place_type<components::Text>, text_, fontSize_, color_, font_),
            });
            return entity;
        }

    private:
        std::string text_;
        int fontSize_{};
        Color color_{colors::White};
        std::optional<awen::sdl::FontHandle> font_;
    };
}
