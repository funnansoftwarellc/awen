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
        auto set_position(glm::vec2 x) -> void
        {
            position_ = x;
        }

        auto get_position() const -> glm::vec2
        {
            return position_;
        }

        auto set_scale(glm::vec2 x) -> void
        {
            scale_ = x;
        }

        auto get_scale() const -> glm::vec2
        {
            return scale_;
        }

        auto set_rotation(float x) -> void
        {
            rotation_ = x;
        }

        auto get_rotation() const -> float
        {
            return rotation_;
        }

        auto set_size(glm::vec2 x) -> void
        {
            size_ = x;
        }

        auto get_size() const -> glm::vec2
        {
            return size_;
        }

        auto synchronize(flecs::entity entity) const -> flecs::entity override
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
        glm::vec2 position_{0.0f, 0.0f};
        glm::vec2 scale_{1.0f, 1.0f};
        glm::vec2 size_{0.0f, 0.0f};
        float rotation_{0.0f};
    };
}
