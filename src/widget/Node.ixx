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
        virtual auto synchronize(flecs::entity entity) -> flecs::entity = 0;
    };
}