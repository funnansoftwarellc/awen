module;

#include <Flecs.hpp>
#include <optional>

export module awen.widget.node;

import awen.core;

export namespace awn::widget
{
    class Node : public awn::core::Object
    {
    public:
        flecs::entity synchronize(flecs::entity entity)
        {
            if (!entity.is_valid())
            {
                entity = awn::core::Engine::instance()->world().entity();
            }

            return entity;
        }

    private:
    };
}