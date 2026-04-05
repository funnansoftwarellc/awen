module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>

export module awen.widget.node2d;
import awen.core.engine;
import awen.widget.node;
import awen.widget.components;

using awen::core::Engine;

export namespace awen::widget
{
    class Node2D : public awen::widget::Node
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

        auto synchronize(flecs::entity entity) -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Transform>({.position = position_});

            return entity;
        }

    private:
        glm::vec2 position_{};
    };

}
