module;

#include <awen/flecs.h>

export module awen.scene;

export import awen.scene.transform;

export namespace awn::scene
{
    /// @brief Flecs module that registers scene components and systems.
    ///
    /// Import once per world via @c world.import<Module>(). The module:
    ///   - Registers Transform and WorldTransform as named components.
    ///   - Registers an observer that zero-initialises WorldTransform whenever Transform
    ///     is added to an entity, so the propagation system always has a writable target.
    ///   - Registers the PropagateWorldTransforms system with cascade-ordered iteration
    ///     (parent-before-child) so each entity accumulates its ancestors' positions correctly.
    struct Module
    {
        /// @brief Constructs the module: registers components, observer, systems, and queries with @p world.
        /// @param world The flecs world to register into.
        explicit Module(flecs::world& world)
        {
            world.module<Module>("awn::scene");

            world.component<Transform>("Transform");
            world.component<WorldTransform>("WorldTransform");

            // Auto-add a zero-initialised WorldTransform the first time Transform is set on
            // an entity, so the propagation system always finds both components present.
            world.observer<Transform>("OnTransformAdded").event(flecs::OnAdd).each([](flecs::entity e, Transform&) { e.set<WorldTransform>({}); });

            // Propagate world transforms parent-to-child each frame via world.progress().
            // The extra (ChildOf, Wildcard) term with cascade() provides parent-before-child
            // iteration ordering without changing how the component terms are resolved.
            // Optional() allows root entities (no ChildOf parent) to still match.
            world.system<const Transform, WorldTransform>("PropagateWorldTransforms")
                .with(flecs::ChildOf, flecs::Wildcard)
                .optional()
                .cascade()
                .each(
                    [](flecs::entity e, const Transform& t, WorldTransform& wt)
                    {
                        const auto parent = e.parent();
                        const auto* parent_wt = parent.is_valid() ? parent.try_get<WorldTransform>() : nullptr;

                        wt.x = t.x + (parent_wt != nullptr ? parent_wt->x : 0.0F);
                        wt.y = t.y + (parent_wt != nullptr ? parent_wt->y : 0.0F);
                    });
        }
    };
}
