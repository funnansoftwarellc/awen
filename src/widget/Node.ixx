module;

#include <awen/flecs/Flecs.hpp>
#include <optional>

export module awen.widget.node;

import awen.core;

export namespace awen::widget
{
    class Node : public awen::core::Object
    {
    public:
        virtual auto synchronize(flecs::entity entity) -> flecs::entity = 0;
    };
}