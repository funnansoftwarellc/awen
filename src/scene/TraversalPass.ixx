module;

#include <cstdint>
#include <unordered_map>

#include <raylib.h>

export module awen.scene.traversal_pass;

import awen.graphics.draw_list;
import awen.scene.scene_nodes;
import awen.scene.texture_cache;
import awen.scene.hierarchy_pool;
import awen.scene.node_pool;
import awen.scene.transform;

export namespace awen::scene
{
    /// @brief Performs a depth-first scene traversal, propagating world transforms and emitting draw commands.
    ///
    /// Visits every node reachable from the hierarchy root in depth-first pre-order (parent
    /// before children, siblings in ascending local_z order). For each node the accumulated
    /// WorldTransform is computed from its parent's WorldTransform and the node's own
    /// Transform (treated as identity {0,0} when no Transform slot exists). Any RectNode,
    /// SpriteNode, or TextNode attached to the same NodeId contributes a corresponding
    /// DrawCommand to @p out.
    ///
    /// @param hierarchy    Scene hierarchy providing the depth-first traversal order.
    /// @param transforms   Per-node local (parent-relative) 2D positions.
    /// @param rect_nodes   Per-node filled-rectangle visual data.
    /// @param circle_nodes Per-node filled-circle visual data.
    /// @param sprite_nodes Per-node sprite visual data.
    /// @param text_nodes   Per-node text label visual data.
    /// @param textures     Cache used to resolve SpriteNode TextureIds to GPU handles.
    /// @param out          DrawList that receives the emitted draw commands.
    /// @note The draw list is not cleared before appending; callers are responsible for
    ///       calling DrawList::clear() when starting a new frame.
    auto build_draw_list(const HierarchyPool& hierarchy, const NodePool<Transform>& transforms, const NodePool<RectNode>& rect_nodes,
                         const NodePool<CircleNode>& circle_nodes, const NodePool<SpriteNode>& sprite_nodes, const NodePool<TextNode>& text_nodes,
                         const TextureCache& textures, awen::graphics::DrawList& out) -> void
    {
        // Maps NodeId.index -> WorldTransform for in-flight parent-to-child propagation.
        auto world_cache = std::unordered_map<uint32_t, WorldTransform>{};

        // Root sentinel carries zero world transform.
        world_cache[hierarchy.root().index] = WorldTransform{};

        hierarchy.depth_first(
            [&](NodeId id)
            {
                const auto* hier = hierarchy.get(id);

                if (hier == nullptr)
                {
                    return;
                }

                // Retrieve parent's accumulated world transform, defaulting to identity.
                const auto parent_wt = [&]() -> WorldTransform
                {
                    if (!hier->parent.is_valid())
                    {
                        return {};
                    }

                    const auto it = world_cache.find(hier->parent.index);

                    if (it == world_cache.end())
                    {
                        return {};
                    }

                    return it->second;
                }();

                const auto* local = transforms.get(id);
                const auto wt = WorldTransform{
                    .x = parent_wt.x + (local != nullptr ? local->x : 0.0F),
                    .y = parent_wt.y + (local != nullptr ? local->y : 0.0F),
                };

                world_cache[id.index] = wt;

                // Emit a DrawRect command if this node carries a RectNode.
                if (const auto* rect = rect_nodes.get(id); rect != nullptr)
                {
                    out.push(awen::graphics::DrawRect{
                        .x = wt.x,
                        .y = wt.y,
                        .width = rect->width,
                        .height = rect->height,
                        .color = rect->color,
                    });
                }

                // Emit a DrawCircle command if this node carries a CircleNode.
                if (const auto* circle = circle_nodes.get(id); circle != nullptr)
                {
                    out.push(awen::graphics::DrawCircle{
                        .center_x = wt.x,
                        .center_y = wt.y,
                        .radius = circle->radius,
                        .color = circle->color,
                    });
                }

                // Emit a DrawSprite command if this node carries a SpriteNode with a valid texture.
                if (const auto* sprite = sprite_nodes.get(id); sprite != nullptr)
                {
                    if (const auto* tex = textures.get(sprite->texture_id); tex != nullptr)
                    {
                        out.push(awen::graphics::DrawSprite{
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
                            .width = sprite->width,
                            .height = sprite->height,
                            .tint = sprite->tint,
                        });
                    }
                }

                // Emit a DrawText command if this node carries a TextNode.
                if (const auto* text = text_nodes.get(id); text != nullptr)
                {
                    out.push(awen::graphics::DrawText{
                        .text = text->text,
                        .x = static_cast<int>(wt.x),
                        .y = static_cast<int>(wt.y),
                        .font_size = text->font_size,
                        .color = text->color,
                    });
                }
            });
    }
}
