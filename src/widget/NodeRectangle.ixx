module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>

#include <memory>
#include <variant>

export module awen.widget.noderectangle;

import awen.core.engine;
import awen.widget.color;
import awen.widget.components;
import awen.widget.node;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Drawable rectangle node synchronized into flecs each frame.
    class NodeRectangle : public Node
    {
    public:
        auto setSize(glm::vec2 x) -> void
        {
            size_ = x;
        }

        [[nodiscard]] auto getSize() const -> glm::vec2
        {
            return size_;
        }

        /// @brief Sets the fill color used when rendering the rectangle.
        /// @param color The rectangle color.
        auto setColor(Color color) -> void
        {
            color_ = color;
        }

        /// @brief Returns the current rectangle fill color.
        /// @return The rectangle color.
        [[nodiscard]] auto getColor() const -> Color
        {
            return color_;
        }

        /// @brief Synchronizes rectangle state into the associated flecs entity.
        /// @param entity The entity to update.
        /// @return The updated entity.
        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Drawable>(components::Drawable{
                .value = std::make_shared<components::DrawableVariant>(std::in_place_type<components::Rectangle>, size_, color_),
            });
            return entity;
        }

    private:
        glm::vec2 size_{};
        Color color_{colors::White};
    };
}