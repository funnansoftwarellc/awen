module;

#include <awen/flecs/Flecs.hpp>
#include <glm/vec2.hpp>

#include <memory>
#include <variant>

export module awen.widget.nodescissor;

import awen.core.engine;
import awen.widget.components;
import awen.widget.node;

using awen::core::Engine;

export namespace awen::widget
{
    /// @brief Pushes a clipping rectangle onto the renderer's scissor stack.
    ///
    /// The rectangle is positioned at the node's world transform and sized
    /// by `setSize`.  Use `NodeScissorEnd` to pop the rectangle.
    class NodeScissorBegin : public Node
    {
    public:
        auto setSize(glm::vec2 size) -> void
        {
            size_ = size;
        }

        [[nodiscard]] auto getSize() const -> glm::vec2
        {
            return size_;
        }

        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Drawable>(components::Drawable{
                .value = std::make_shared<components::DrawableVariant>(std::in_place_type<components::ScissorBegin>, size_),
            });
            return entity;
        }

    private:
        glm::vec2 size_{};
    };

    /// @brief Pops the most recent clipping rectangle off the scissor stack.
    class NodeScissorEnd : public Node
    {
    public:
        [[nodiscard]] auto synchronize(flecs::entity entity) const -> flecs::entity override
        {
            if (!entity.is_valid())
            {
                entity = Engine::instance()->world().entity();
            }

            entity.set<components::Drawable>(components::Drawable{
                .value = std::make_shared<components::DrawableVariant>(std::in_place_type<components::ScissorEnd>),
            });
            return entity;
        }
    };
}
