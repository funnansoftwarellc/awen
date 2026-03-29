module;

#include <algorithm>
#include <string>
#include <vector>

#include <awen/flecs.h>

export module awen.scene;

export import awen.scene.transform;
export import awen.scene.texture_cache;

import awen.graphics.draw_list;
import awen.graphics.draw_components;

export namespace awn::scene
{
    /// @brief Draw order for a scene entity — lower z values are rendered first among siblings.
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
    ///   - Registers the PropagateWorldTransforms system with three terms (own Transform, mutable
    ///     WorldTransform, and parent WorldTransform via cascade-ordered optional traversal), so
    ///   - Pre-builds a cached query for top-level scene entities (those with Transform
    ///     but no ChildOf relationship), used by build_draw_list().
    struct SceneModule
    {
        /// @brief Cached query for entities that are roots — have Transform but no parent.
        flecs::query<const Transform> roots_query;

        /// @brief Constructs the module: registers components, observer, and systems with @p world.
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

            // Pre-build the query that seeds the draw-list DFS traversal with the set of
            // top-level scene entities (have Transform, no ChildOf parent).
            roots_query = world.query_builder<const Transform>().without(flecs::ChildOf, flecs::Wildcard).build();
        }
    };

    /// @brief Performs a depth-first, z-ordered traversal of all scene entities and appends
    ///        draw commands to @p out.
    ///
    /// Reads the WorldTransform component written by the PropagateWorldTransforms system
    /// (call @c world.progress() before this function each frame). The traversal seeds itself
    /// from entities with Transform and no parent, so no root sentinel entity is required.
    /// The list is not cleared before appending; callers must call DrawList::clear() first.
    ///
    /// @param world    The flecs world containing all scene entities; must have SceneModule imported.
    /// @param textures Texture cache used to resolve DrawSprite texture ids to GPU handles.
    /// @param out      DrawList that receives the emitted render commands.
    auto build_draw_list(flecs::world& world, TextureCache& textures, awn::graphics::DrawList& out) -> void;

    // ── build_draw_list definition ────────────────────────────────────────────

    inline auto build_draw_list(flecs::world& world, TextureCache& textures, awn::graphics::DrawList& out) -> void
    {
        // Z-ordered DFS. WorldTransforms have already been stamped by PropagateWorldTransforms.
        // Entities without a WorldTransform component are silently skipped.
        auto stack = std::vector<flecs::entity>{};

        // Pushes the z-sorted children of @p parent onto the stack in descending order so
        // the lowest-z child sits on top and is popped (processed) first.
        const auto push_children = [&](flecs::entity parent)
        {
            auto children = std::vector<std::pair<int, flecs::entity>>{};

            parent.children(
                [&](flecs::entity child)
                {
                    if (child.has<DrawOrder>())
                    {
                        const auto* order = child.try_get<DrawOrder>();
                        children.emplace_back(order != nullptr ? order->z : 0, child);
                    }
                });

            std::ranges::sort(children, [](const auto& a, const auto& b) { return a.first > b.first; });

            for (auto& [z, child] : children)
            {
                stack.push_back(child);
            }
        };

        // Seed the DFS from top-level scene entities, z-sorted among themselves.
        const auto& mod = world.get<SceneModule>();

        auto roots = std::vector<std::pair<int, flecs::entity>>{};

        mod.roots_query.each(
            [&](flecs::entity e, const Transform&)
            {
                if (e.has<DrawOrder>())
                {
                    const auto* order = e.try_get<DrawOrder>();
                    roots.emplace_back(order != nullptr ? order->z : 0, e);
                }
            });

        std::ranges::sort(roots, [](const auto& a, const auto& b) { return a.first > b.first; });

        for (auto& [z, e] : roots)
        {
            stack.push_back(e);
        }

        while (!stack.empty())
        {
            const auto node = stack.back();
            stack.pop_back();

            const auto* wt = node.try_get<WorldTransform>();

            if (wt != nullptr)
            {
                if (const auto* rect = node.try_get<awn::graphics::DrawRect>(); rect != nullptr)
                {
                    out.push(awn::graphics::RenderRect{
                        .x = wt->x,
                        .y = wt->y,
                        .width = rect->width,
                        .height = rect->height,
                        .color = rect->color,
                    });
                }

                if (const auto* circle = node.try_get<awn::graphics::DrawCircle>(); circle != nullptr)
                {
                    out.push(awn::graphics::RenderCircle{
                        .x = wt->x,
                        .y = wt->y,
                        .radius = circle->radius,
                        .color = circle->color,
                    });
                }

                if (const auto* sprite = node.try_get<awn::graphics::DrawSprite>(); sprite != nullptr)
                {
                    if (const auto* tex = textures.get(sprite->texture_id); tex != nullptr)
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
                            .x = wt->x,
                            .y = wt->y,
                            .width = sprite->width,
                            .height = sprite->height,
                            .tint = sprite->tint,
                        });
                    }
                }

                if (const auto* text = node.try_get<awn::graphics::DrawText>(); text != nullptr)
                {
                    out.push(awn::graphics::RenderText{
                        .text = text->text,
                        .x = static_cast<int>(wt->x),
                        .y = static_cast<int>(wt->y),
                        .font_size = text->font_size,
                        .color = text->color,
                    });
                }
            }

            push_children(node);
        }
    }
}
