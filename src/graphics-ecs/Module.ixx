module;

#include <awen/flecs.h>
#include <raylib.h>

export module awen.graphics.ecs;
export import awen.graphics.ecs.components;
export import awen.core.ecs;

export namespace awn::graphics::ecs
{
    /// @brief Flecs module that registers graphics ECS components and initialization systems.
    struct Module
    {
        /// @brief Constructs the module and registers all graphics ECS types and systems.
        /// @param world The flecs world to register into.
        explicit Module(flecs::world& world)
        {
            world.module<Module>("graphics.ecs");

            world.component<Color>("Color");
            world.component<DrawRect>("DrawRect");
            world.component<DrawCircle>("DrawCircle");
            world.component<DrawText>("DrawText");
            world.component<Window>("Window");

            // Initialize the native raylib window whenever a Window component is set.
            world.observer<Window>()
                .event(flecs::OnSet)
                .each(
                    [](flecs::entity entity, const Window& component)
                    {
                        if (!IsWindowReady())
                        {
                            auto combined = 0U;

                            for (const auto flag : component.flags)
                            {
                                combined |= static_cast<unsigned int>(flag);
                            }

                            SetConfigFlags(combined);
                            InitWindow(component.width, component.height, component.title.c_str());
                        }

                        entity.set<WindowRuntime>({
                            .created = IsWindowReady(),
                            .should_close = IsWindowReady() && WindowShouldClose(),
                        });
                    });

            world.observer<Window>()
                .event(flecs::OnRemove)
                .each(
                    [](flecs::entity, const Window&)
                    {
                        if (IsWindowReady())
                        {
                            CloseWindow();
                        }
                    });

            // Keep WindowRuntime synchronized with the native window close state.
            world.system<awn::core::RunState>().kind<awn::core::ecs::phases::OnEvent>().each(
                [](awn::core::RunState& run_state)
                {
                    PollInputEvents();

                    if (WindowShouldClose())
                    {
                        run_state.type = awn::core::RunState::Type::stopping;
                    }
                });
        }
    };
}