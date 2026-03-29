module;

#include <awen/flecs.h>

export module awen.graphics;

export import awen.graphics.event;
export import awen.graphics.color;
export import awen.graphics.texture_id;
export import awen.graphics.draw_components;
export import awen.graphics.draw_list;
export import awen.graphics.window;
export import awen.graphics.renderer;

export namespace awn::graphics
{
    /// @brief Flecs module that registers graphics components.
    struct Module
    {
        explicit Module(flecs::world& world)
        {
            world.module<Module>("awn::graphics");

            world.component<DrawRect>("DrawRect");
            world.component<DrawCircle>("DrawCircle");
            world.component<DrawText>("DrawText");
            world.component<DrawSprite>("DrawSprite");
        }
    };
}
