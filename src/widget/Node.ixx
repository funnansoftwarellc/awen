module;

#include <awen/flecs/Flecs.hpp>
#include <optional>

export module awen.widget.node;

import awen.core;

export namespace awen::widget
{
    /// @brief Thhe Node class represents a base transformable or drawable object.
    class Node : public awen::core::Object
    {
    public:
        /// @brief This function is invoked on the render thread to copy the contents of this object to a flecs entity.
        /// @param entity The entity that represents the drawable and transformable components of this node.
        /// @return An instantiated entity if this node requires transforming or drawing.
        [[nodiscard]] virtual auto synchronize(flecs::entity entity) const -> flecs::entity = 0;
    };
}