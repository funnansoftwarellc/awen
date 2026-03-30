module;

#include <awen/flecs.h>

export module awen.core.ecs;

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

    /// @brief Flecs module that registers core ECS components.
    struct Module
    {
        /// @brief Constructs the module and registers all core phase components.
        /// @param world The flecs world to register into.
        explicit Module(flecs::world& world)
        {
            world.module<Module>("core.ecs");

            world.component<OnUpdate>("OnUpdate");
            world.component<OnUpdatePhysics>("OnUpdatePhysics");
            world.component<OnRender>("OnRender");
        }
    };
}