module;

#include <awen/flecs/Flecs.hpp>

#include <memory>
#include <variant>

export module awen.widget.nodecircle;

import awen.core.engine;
import awen.widget.color;
import awen.widget.components;
import awen.widget.node;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Drawable circle node synchronized into flecs each frame.
    class NodeCircle : public Node
    {
    public:
        /// @brief Sets the circle radius.
        /// @param radius The radius in logical pixels.
        auto setRadius(float radius) -> void
        {
            radius_ = radius;
        }

        /// @brief Returns the current radius.
        /// @return The radius in logical pixels.
        [[nodiscard]] auto getRadius() const -> float
        {
            return radius_;
        }

        /// @brief Sets the fill color used when rendering the circle.
        /// @param color The circle color.
        auto setColor(Color color) -> void
        {
            color_ = color;
        }

        /// @brief Returns the current circle fill color.
        /// @return The circle color.
        [[nodiscard]] auto getColor() const -> Color
        {
            return color_;
        }

        /// @brief Synchronizes circle state into the associated flecs entity.
        /// @param entity The entity to update.
        /// @return The updated entity.
        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Drawable>(components::Drawable{
                .value = std::make_shared<components::DrawableVariant>(std::in_place_type<components::Circle>, radius_, color_),
            });
            return entity;
        }

    private:
        float radius_{};
        Color color_{colors::White};
    };
}