module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>

export module awen.widget.control;

import awen.core.engine;
import awen.widget.components;
import awen.widget.node;

export namespace awen::widget
{
    class Control : public Node
    {
    public:
        auto setPosition(glm::vec2 x) -> void
        {
            position_ = x;
        }

        [[nodiscard]] auto getPosition() const -> glm::vec2
        {
            return position_;
        }

        auto setScale(glm::vec2 x) -> void
        {
            scale_ = x;
        }

        [[nodiscard]] auto getScale() const -> glm::vec2
        {
            return scale_;
        }

        auto setRotation(float x) -> void
        {
            rotation_ = x;
        }

        [[nodiscard]] auto getRotation() const -> float
        {
            return rotation_;
        }

        auto setSize(glm::vec2 x) -> void
        {
            size_ = x;
        }

        [[nodiscard]] auto getSize() const -> glm::vec2
        {
            return size_;
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            using awen::core::Engine;
            using awen::widget::components::Transform;

            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<Transform>({
                .position = position_,
                .scale = scale_,
                .rotation = rotation_,
            });

            return entity;
        }

    private:
        glm::vec2 position_{0.0F, 0.0F};
        glm::vec2 scale_{1.0F, 1.0F};
        glm::vec2 size_{0.0F, 0.0F};
        float rotation_{0.0F};
    };
}
