module;

#include <awen/flecs.h>

export module awen.core;

export import awen.overloaded;
export import awen.signal;

export namespace awn::core
{
    /// @brief Frame update marker component.
    struct OnUpdate
    {
    };

    /// @brief Physics update marker component.
    struct OnUpdatePhysics
    {
    };

    /// @brief Render update marker component.
    struct OnRender
    {
    };

    /// @brief Flecs module that registers core components.
    struct Module
    {
        explicit Module(flecs::world& world)
        {
            world.module<Module>("awn::core");
            world.component<OnUpdate>("OnUpdate");
            world.component<OnUpdatePhysics>("OnUpdatePhysics");
            world.component<OnRender>("OnRender");
        }
    };
}
