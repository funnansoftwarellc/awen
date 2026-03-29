module;

#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <flecs.h>

export module awen.scene;

export import awen.scene.transform;
export import awen.scene.texture_id;
export import awen.scene.texture_cache;
export import awen.scene.scene_nodes;

import awen.graphics.draw_list;

export namespace awn::scene
{
    /// @brief Draw order for a scene node — lower z values are rendered first among siblings.
    struct DrawOrder
    {
        int z{};
    };

    /// @brief Scene graph backed by a flecs ECS world.
    ///
    /// Entities are the first-class handles; use scene.root() as the common
    /// ancestor and scene.add_child<T>() to grow the hierarchy. Component data
    /// (Transform, DrawOrder, RectNode, etc.) is set directly on the returned
    /// flecs::entity via entity.set<T>({...}).
    ///
    /// build_draw_list() is the render system: it performs a depth-first
    /// traversal of the entity hierarchy, propagates world transforms
    /// parent-to-child, and emits a DrawCommand for every entity that carries a
    /// visual component (RectNode, CircleNode, SpriteNode, TextNode). Siblings
    /// are always visited in ascending DrawOrder::z order.
    ///
    /// Usage:
    /// @code
    ///   auto scene = awn::scene::Scene{};
    ///   auto root  = scene.root();
    ///   auto rect  = scene.add_child<RectNode>(root)
    ///                    .set<Transform>({.x = 10.0F, .y = 20.0F})
    ///                    .set<RectNode>({.width = 100.0F, .height = 50.0F,
    ///                                   .color  = awn::graphics::Color{255, 0, 0, 255}});
    ///   auto dl = awn::graphics::DrawList{};
    ///   scene.build_draw_list(dl);
    /// @endcode
    class Scene
    {
    public:
        Scene();

        ~Scene() = default;

        // Non-copyable — flecs::world and TextureCache own exclusive resources.
        Scene(const Scene&) = delete;
        auto operator=(const Scene&) -> Scene& = delete;

        Scene(Scene&&) = default;
        auto operator=(Scene&&) -> Scene& = default;

        /// @brief Returns the invisible root sentinel entity.
        ///
        /// The root is never emitted as a draw command; it exists solely as the
        /// common ancestor for all top-level scene entities.
        [[nodiscard]] auto root() const noexcept -> flecs::entity;

        /// @brief Creates a child entity under @p parent and attaches it to the hierarchy.
        ///
        /// Every entity receives a default-constructed Transform and a DrawOrder
        /// component. When T is a concrete visual type (RectNode, CircleNode,
        /// SpriteNode, or TextNode) a default-constructed T component is also
        /// added. When T is void the entity acts as a pure group/container node.
        ///
        /// @tparam T  Visual component type, or void for a group node.
        /// @param parent   Parent entity; pass scene.root() for top-level nodes.
        /// @param local_z  Draw order relative to siblings — lower values first.
        /// @return The newly created flecs::entity ready for component mutation.
        template <typename T>
        [[nodiscard]] auto add_child(flecs::entity parent, int local_z = 0) -> flecs::entity;

        /// @brief Render system — performs a depth-first traversal and appends draw commands to @p out.
        ///
        /// World transforms are propagated from parent to child during the walk.
        /// Siblings are visited in ascending DrawOrder::z order. The list is not
        /// cleared before appending; callers must call DrawList::clear() at the
        /// start of each frame.
        ///
        /// @param out DrawList that receives the emitted commands.
        auto build_draw_list(awn::graphics::DrawList& out) -> void;

        /// @brief Loads a texture from @p path or returns the cached TextureId if already loaded.
        /// @param path File path of the image to load.
        /// @return TextureId that can be passed to a SpriteNode component.
        [[nodiscard]] auto load_texture(const std::string& path) -> TextureId;

        /// @brief Returns the underlying flecs world for direct ECS access.
        [[nodiscard]] auto raw_world() noexcept -> flecs::world&;

    private:
        flecs::world world_;

        // Store the root entity id separately so it remains valid across moves.
        flecs::entity_t root_id_{};

        TextureCache textures_;
    };

    // ── add_child<T> ─────────────────────────────────────────────────────────

    template <typename T>
    auto Scene::add_child(flecs::entity parent, int local_z) -> flecs::entity
    {
        auto e = world_.entity().child_of(parent).set<Transform>({}).set<DrawOrder>({.z = local_z});

        if constexpr (!std::is_void_v<T>)
        {
            e.set<T>({});
        }

        return e;
    }

    // ── Scene method definitions ──────────────────────────────────────────────

    Scene::Scene() : root_id_{world_.entity("root").id()}
    {
    }

    auto Scene::root() const noexcept -> flecs::entity
    {
        return world_.entity(root_id_);
    }

    auto Scene::raw_world() noexcept -> flecs::world&
    {
        return world_;
    }

    auto Scene::load_texture(const std::string& path) -> TextureId
    {
        return textures_.load(path);
    }

    auto Scene::build_draw_list(awn::graphics::DrawList& out) -> void
    {
        // Iterative depth-first traversal driven by a stack of (entity, parent world transform) frames.
        struct Frame
        {
            flecs::entity node;
            WorldTransform parent_wt;
        };

        auto stack = std::vector<Frame>{};

        // Collects the immediate children of parent, sorts them in descending DrawOrder::z,
        // and pushes them onto the stack so the lowest-z child sits on top and is processed first.
        const auto push_children = [&](flecs::entity parent, WorldTransform wt)
        {
            auto children = std::vector<std::pair<int, flecs::entity>>{};

            parent.children(
                [&](flecs::entity child)
                {
                    const auto* order = child.try_get<DrawOrder>();
                    children.emplace_back(order != nullptr ? order->z : 0, child);
                });

            std::ranges::sort(children,
                              [](const auto& a, const auto& b)
                              {
                                  // Descending: lowest-z lands on top of the stack and is popped first.
                                  return a.first > b.first;
                              });

            for (auto& [z, child] : children)
            {
                stack.push_back(Frame{.node = child, .parent_wt = wt});
            }
        };

        push_children(world_.entity(root_id_), WorldTransform{});

        while (!stack.empty())
        {
            auto [node, parent_wt] = stack.back();
            stack.pop_back();

            const auto* local = node.try_get<Transform>();

            const auto wt = WorldTransform{
                .x = parent_wt.x + (local != nullptr ? local->x : 0.0F),
                .y = parent_wt.y + (local != nullptr ? local->y : 0.0F),
            };

            // Emit a draw command for whichever visual component this entity carries.
            if (const auto* rect = node.try_get<RectNode>(); rect != nullptr)
            {
                out.push(awn::graphics::DrawRect{
                    .x = wt.x,
                    .y = wt.y,
                    .width = rect->width,
                    .height = rect->height,
                    .color = rect->color,
                });
            }

            if (const auto* circle = node.try_get<CircleNode>(); circle != nullptr)
            {
                out.push(awn::graphics::DrawCircle{
                    .center_x = wt.x,
                    .center_y = wt.y,
                    .radius = circle->radius,
                    .color = circle->color,
                });
            }

            if (const auto* sprite = node.try_get<SpriteNode>(); sprite != nullptr)
            {
                if (const auto* tex = textures_.get(sprite->texture_id); tex != nullptr)
                {
                    out.push(awn::graphics::DrawSprite{
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

            if (const auto* text = node.try_get<TextNode>(); text != nullptr)
            {
                out.push(awn::graphics::DrawText{
                    .text = text->text,
                    .x = static_cast<int>(wt.x),
                    .y = static_cast<int>(wt.y),
                    .font_size = text->font_size,
                    .color = text->color,
                });
            }

            push_children(node, wt);
        }
    }
}
