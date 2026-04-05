module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>

export module awen.widget.nodetransform;
import awen.core.engine;
import awen.widget.node;
import awen.widget.components;

using awen::core::Engine;

export namespace awen::widget
{
    class NodeTransform : public awen::widget::Node
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

        auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Transform>({
                .position = position_,
                .scale = scale_,
                .rotation = rotation_,
            });

            return entity;
        }

    private:
        glm::vec2 position_{};
        glm::vec2 scale_{1.0F, 1.0F};
        float rotation_{0.0F};
    };

}
