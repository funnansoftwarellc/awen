module;

#include <awen/flecs/Flecs.hpp>

export module awen.sdl.phases;

export namespace awen::sdl::phases
{
    /// @brief Phase tag: pump SDL events, update timing, refresh input singletons.
    struct OnEvent
    {
    };

    /// @brief Phase tag: per-frame variable-delta game logic.
    struct OnUpdate
    {
    };

    /// @brief Phase tag: physics / collision step.
    /// @note Currently runs once per frame with the variable frame delta.
    ///       Fixed-step orchestration is a planned follow-up.
    struct OnPhysics
    {
    };

    /// @brief Phase tag: propagate transforms, build and sort the draw list.
    struct OnPreRender
    {
    };

    /// @brief Phase tag: dispatch the draw list to SDL_Renderer.
    struct OnRender
    {
    };

    /// @brief Phase tag: post-render bookkeeping (frame counters, debug overlays).
    struct OnPostRender
    {
    };

    /// @brief Register all awen phase tags as flecs phases layered on built-in phases.
    /// @param world The flecs world.
    auto registerAll(flecs::world& world) -> void
    {
        world.component<OnEvent>().add(flecs::Phase).depends_on(flecs::OnLoad);
        world.component<OnUpdate>().add(flecs::Phase).depends_on(flecs::OnUpdate);
        world.component<OnPhysics>().add(flecs::Phase).depends_on(flecs::PostUpdate);
        world.component<OnPreRender>().add(flecs::Phase).depends_on(flecs::PreStore);
        world.component<OnRender>().add(flecs::Phase).depends_on(flecs::OnStore);
        world.component<OnPostRender>().add(flecs::Phase).depends_on(world.entity<OnRender>());
    }
}
