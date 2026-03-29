module;

#include <string>

#include <awen/flecs.h>

export module awen.scene;

export import awen.scene.transform;
export import awen.scene.texture_cache;

import awen.graphics.draw_list;
import awen.graphics.draw_components;

export namespace awn::scene
{
    /// @brief Global draw order — lower z values are rendered before higher z values within
    ///        each visual archetype (DrawRect, DrawCircle, DrawText, DrawSprite).
    struct DrawOrder
    {
        int z{};
    };

    /// @brief Flecs module that registers scene components and systems.
    ///
    /// Import once per world via @c world.import<SceneModule>(). The module:
    ///   - Registers Transform, WorldTransform, and DrawOrder as named components.
    ///   - Registers an observer that zero-initialises WorldTransform whenever Transform
    ///     is added to an entity, so the propagation system always has a writable target.
    ///   - Registers the PropagateWorldTransforms system with cascade-ordered iteration
    ///     (parent-before-child) so each entity accumulates its ancestors' positions correctly.
    ///   - Pre-builds per-archetype z-sorted queries (DrawRect, DrawCircle, DrawText, DrawSprite)
    ///     used by build_draw_list() to emit render commands without heap-allocated traversal.
    struct SceneModule
    {
        /// @brief Pre-built query that emits all rect entities in ascending DrawOrder.z order.
        flecs::query<const WorldTransform, const awn::graphics::DrawRect, const DrawOrder> rect_query;

        /// @brief Pre-built query that emits all circle entities in ascending DrawOrder.z order.
        flecs::query<const WorldTransform, const awn::graphics::DrawCircle, const DrawOrder> circle_query;

        /// @brief Pre-built query that emits all text entities in ascending DrawOrder.z order.
        flecs::query<const WorldTransform, const awn::graphics::DrawText, const DrawOrder> text_query;

        /// @brief Pre-built query that emits all sprite entities in ascending DrawOrder.z order.
        flecs::query<const WorldTransform, const awn::graphics::DrawSprite, const DrawOrder> sprite_query;

        /// @brief Constructs the module: registers components, observer, systems, and queries with @p world.
        /// @param world The flecs world to register into.
        explicit SceneModule(flecs::world& world)
        {
            world.module<SceneModule>("awn::scene");

            world.component<Transform>("Transform");
            world.component<WorldTransform>("WorldTransform");
            world.component<DrawOrder>("DrawOrder");

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

            // Ascending comparator shared by all per-archetype queries below.
            const auto ascending_z = [](flecs::entity_t, const DrawOrder* a, flecs::entity_t, const DrawOrder* b) -> int
            { return (a->z > b->z) - (a->z < b->z); };

            rect_query =
                world.query_builder<const WorldTransform, const awn::graphics::DrawRect, const DrawOrder>().order_by<DrawOrder>(ascending_z).build();

            circle_query = world.query_builder<const WorldTransform, const awn::graphics::DrawCircle, const DrawOrder>()
                               .order_by<DrawOrder>(ascending_z)
                               .build();

            text_query =
                world.query_builder<const WorldTransform, const awn::graphics::DrawText, const DrawOrder>().order_by<DrawOrder>(ascending_z).build();

            sprite_query = world.query_builder<const WorldTransform, const awn::graphics::DrawSprite, const DrawOrder>()
                               .order_by<DrawOrder>(ascending_z)
                               .build();
        }
    };

    /// @brief Iterates all visual entities via per-archetype pre-built queries and appends
    ///        render commands to @p out.
    ///
    /// Within each archetype, commands are emitted in ascending DrawOrder.z order.
    /// Archetypes are emitted in the fixed sequence: DrawRect, DrawCircle, DrawText, DrawSprite.
    /// Reads the WorldTransform component written by the PropagateWorldTransforms system —
    /// call @c world.progress() before this function each frame.
    /// The list is not cleared before appending; callers must call DrawList::clear() first.
    ///
    /// @param world    The flecs world containing all scene entities; must have SceneModule imported.
    /// @param textures Texture cache used to resolve DrawSprite texture ids to GPU handles.
    /// @param out      DrawList that receives the emitted render commands.
    auto build_draw_list(flecs::world& world, TextureCache& textures, awn::graphics::DrawList& out) -> void;

    // ── build_draw_list definition ────────────────────────────────────────────

    inline auto build_draw_list(flecs::world& world, TextureCache& textures, awn::graphics::DrawList& out) -> void
    {
        const auto& mod = world.get<SceneModule>();

        mod.rect_query.each(
            [&](const WorldTransform& wt, const awn::graphics::DrawRect& rect, const DrawOrder&)
            {
                out.push(awn::graphics::RenderRect{
                    .x = wt.x,
                    .y = wt.y,
                    .width = rect.width,
                    .height = rect.height,
                    .color = rect.color,
                });
            });

        mod.circle_query.each(
            [&](const WorldTransform& wt, const awn::graphics::DrawCircle& circle, const DrawOrder&)
            {
                out.push(awn::graphics::RenderCircle{
                    .x = wt.x,
                    .y = wt.y,
                    .radius = circle.radius,
                    .color = circle.color,
                });
            });

        mod.text_query.each(
            [&](const WorldTransform& wt, const awn::graphics::DrawText& text, const DrawOrder&)
            {
                out.push(awn::graphics::RenderText{
                    .text = text.text,
                    .x = static_cast<int>(wt.x),
                    .y = static_cast<int>(wt.y),
                    .font_size = text.font_size,
                    .color = text.color,
                });
            });

        mod.sprite_query.each(
            [&](const WorldTransform& wt, const awn::graphics::DrawSprite& sprite, const DrawOrder&)
            {
                if (const auto* tex = textures.get(sprite.texture_id); tex != nullptr)
                {
                    out.push(awn::graphics::RenderSprite{
                        .texture =
                            {
                                .id = tex->id,
                                .width = tex->width,
                                .height = tex->height,
                                .mipmaps = tex->mipmaps,
                                .format = tex->format,
                            },
                        .x = wt.x,
                        .y = wt.y,
                        .width = sprite.width,
                        .height = sprite.height,
                        .tint = sprite.tint,
                    });
                }
            });
    }
}
