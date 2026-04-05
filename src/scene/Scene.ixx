module;

#include <concepts>
#include <string>
#include <string_view>

export module awen.scene;

export import awen.scene.node_id;
export import awen.scene.node_pool;
export import awen.scene.hierarchy_pool;
export import awen.scene.transform;
export import awen.scene.texture_id;
export import awen.scene.texture_cache;
export import awen.scene.scene_nodes;
export import awen.scene.traversal_pass;

import awen.graphics.draw_list;

export namespace awen::scene
{
    /// @brief Forward declaration — full definition follows below.
    class Scene;

    /// @brief Copyable handle to a typed node in a Scene.
    ///
    /// Holds a NodeId and a non-owning pointer back to the owning Scene. All
    /// mutating operations (set_transform, set_size, …) write directly into the
    /// Scene's pools, so changes are immediately visible to the next
    /// Scene::build_draw_list() call.
    ///
    /// @tparam T The data-component type associated with this node: RectNode,
    ///           SpriteNode, TextNode, or void for a pure hierarchy anchor.
    template <typename T>
    class NodeHandle
    {
    public:
        /// @brief Constructs a handle wrapping @p id and pointing at @p scene.
        NodeHandle(NodeId id, Scene* scene) noexcept : id_{id}, scene_{scene}
        {
        }

        /// @brief Returns the NodeId of the underlying hierarchy slot.
        [[nodiscard]] auto node_id() const noexcept -> NodeId
        {
            return id_;
        }

        /// @brief Allocates a new child node of type U and returns a handle to it.
        /// @param local_z Draw order relative to siblings; lower values are visited first.
        template <typename U>
        [[nodiscard]] auto add_child(int local_z = 0) const -> NodeHandle<U>;

        /// @brief Sets the local (parent-relative) position of this node.
        /// @param t New Transform to write into the transforms pool.
        auto set_transform(Transform t) const -> const NodeHandle<T>&;

        /// @brief Sets the width and height of a RectNode or SpriteNode.
        /// @param width  New width in pixels.
        /// @param height New height in pixels.
        auto set_size(float width, float height) const -> const NodeHandle<T>&
            requires(std::same_as<T, RectNode> || std::same_as<T, SpriteNode>);

        /// @brief Sets the fill colour of a RectNode or CircleNode, or the text colour of a TextNode.
        /// @param color New colour value.
        auto set_color(graphics::Color color) const -> const NodeHandle<T>&
            requires(std::same_as<T, RectNode> || std::same_as<T, CircleNode> || std::same_as<T, TextNode>);

        /// @brief Sets the radius of a CircleNode.
        /// @param radius New radius in pixels.
        auto set_radius(float radius) const -> const NodeHandle<T>&
            requires std::same_as<T, CircleNode>;

        /// @brief Sets the TextureId on a SpriteNode.
        /// @param id TextureId returned by Scene::load_texture().
        auto set_texture(TextureId id) const -> const NodeHandle<T>&
            requires std::same_as<T, SpriteNode>;

        /// @brief Sets the tint colour multiplied with the sprite's texture.
        /// @param tint New tint colour value.
        auto set_tint(graphics::Color tint) const -> const NodeHandle<T>&
            requires std::same_as<T, SpriteNode>;

        /// @brief Sets the display text of a TextNode.
        /// @param text String to display.
        auto set_text(std::string_view text) const -> const NodeHandle<T>&
            requires std::same_as<T, TextNode>;

        /// @brief Sets the font size of a TextNode.
        /// @param size Font size in pixels.
        auto set_font_size(int size) const -> const NodeHandle<T>&
            requires std::same_as<T, TextNode>;

    private:
        NodeId id_{};
        Scene* scene_{};
    };

    /// @brief Owns the full scene graph: hierarchy, component pools, and texture cache.
    ///
    /// Every node in the scene has an entry in the hierarchy pool and the
    /// transforms pool. Nodes of a specific visual type (RectNode, SpriteNode,
    /// TextNode) additionally have an entry in the corresponding data pool.
    /// All pool allocations use the same index space so that the depth-first
    /// traversal pass can look up each component by the same NodeId.
    ///
    /// Usage:
    /// @code
    ///   auto scene = awen::scene::Scene{};
    ///   auto root  = scene.root();
    ///   auto rect  = root.add_child<awen::scene::RectNode>()
    ///                    .set_transform({.x = 10.0F, .y = 20.0F})
    ///                    .set_size(100.0F, 50.0F)
    ///                    .set_color(awen::graphics::Color{255, 0, 0, 255});
    ///   auto dl = awen::graphics::DrawList{};
    ///   scene.build_draw_list(dl);
    /// @endcode
    class Scene
    {
    public:
        Scene();

        /// @brief Default.
        ~Scene() = default;

        // Non-copyable (TextureCache owns GPU resources).
        Scene(const Scene&) = delete;
        auto operator=(const Scene&) -> Scene& = delete;

        Scene(Scene&&) = default;
        auto operator=(Scene&&) -> Scene& = default;

        /// @brief Returns a handle to the invisible root sentinel node.
        ///
        /// The root itself is never visited by the traversal pass; it exists
        /// purely as the common ancestor for all top-level scene nodes.
        [[nodiscard]] auto root() noexcept -> NodeHandle<void>;

        /// @brief Runs the depth-first traversal pass, appending draw commands to @p out.
        ///
        /// The list is not cleared before appending; call DrawList::clear() at
        /// the start of each frame before calling this method.
        /// @param out DrawList that receives the emitted commands.
        auto build_draw_list(awen::graphics::DrawList& out) const -> void;

        /// @brief Loads a texture from @p path or returns the cached TextureId if already loaded.
        /// @param path File path of the image to load.
        /// @return TextureId that can be assigned to a SpriteNode via NodeHandle::set_texture().
        [[nodiscard]] auto load_texture(const std::string& path) -> TextureId;

    private:
        template <typename T>
        friend class NodeHandle;

        HierarchyPool hierarchy_;
        NodePool<Transform> transforms_;
        NodePool<RectNode> rects_;
        NodePool<CircleNode> circles_;
        NodePool<SpriteNode> sprites_;
        NodePool<TextNode> texts_;
        TextureCache textures_;

        /// @brief Core allocation helper called by NodeHandle::add_child<T>().
        ///
        /// Allocates a hierarchy slot and a transforms slot, then allocates the
        /// type-specific data slot when T is a concrete visual node type.
        /// @param parent  NodeId of the parent hierarchy node.
        /// @param local_z Draw order relative to siblings.
        /// @return NodeHandle<T> wrapping the freshly allocated NodeId.
        template <typename T>
        [[nodiscard]] auto add_child_node(NodeId parent, int local_z) -> NodeHandle<T>;
    };

    // ── NodeHandle<T> method definitions ──────────────────────────────────────

    template <typename T>
    template <typename U>
    auto NodeHandle<T>::add_child(int local_z) const -> NodeHandle<U>
    {
        return scene_->add_child_node<U>(id_, local_z);
    }

    template <typename T>
    auto NodeHandle<T>::set_transform(Transform t) const -> const NodeHandle<T>&
    {
        if (auto* xf = scene_->transforms_.get(id_); xf != nullptr)
        {
            *xf = t;
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_size(float width, float height) const -> const NodeHandle<T>&
        requires(std::same_as<T, RectNode> || std::same_as<T, SpriteNode>)
    {
        if constexpr (std::same_as<T, RectNode>)
        {
            if (auto* r = scene_->rects_.get(id_); r != nullptr)
            {
                r->width = width;
                r->height = height;
            }
        }
        else
        {
            if (auto* s = scene_->sprites_.get(id_); s != nullptr)
            {
                s->width = width;
                s->height = height;
            }
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_color(graphics::Color color) const -> const NodeHandle<T>&
        requires(std::same_as<T, RectNode> || std::same_as<T, CircleNode> || std::same_as<T, TextNode>)
    {
        if constexpr (std::same_as<T, RectNode>)
        {
            if (auto* r = scene_->rects_.get(id_); r != nullptr)
            {
                r->color = color;
            }
        }
        else if constexpr (std::same_as<T, CircleNode>)
        {
            if (auto* c = scene_->circles_.get(id_); c != nullptr)
            {
                c->color = color;
            }
        }
        else
        {
            if (auto* tn = scene_->texts_.get(id_); tn != nullptr)
            {
                tn->color = color;
            }
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_radius(float radius) const -> const NodeHandle<T>&
        requires std::same_as<T, CircleNode>
    {
        if (auto* c = scene_->circles_.get(id_); c != nullptr)
        {
            c->radius = radius;
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_texture(TextureId id) const -> const NodeHandle<T>&
        requires std::same_as<T, SpriteNode>
    {
        if (auto* s = scene_->sprites_.get(id_); s != nullptr)
        {
            s->texture_id = id;
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_tint(graphics::Color tint) const -> const NodeHandle<T>&
        requires std::same_as<T, SpriteNode>
    {
        if (auto* s = scene_->sprites_.get(id_); s != nullptr)
        {
            s->tint = tint;
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_text(std::string_view text) const -> const NodeHandle<T>&
        requires std::same_as<T, TextNode>
    {
        if (auto* tn = scene_->texts_.get(id_); tn != nullptr)
        {
            tn->text = text;
        }

        return *this;
    }

    template <typename T>
    auto NodeHandle<T>::set_font_size(int size) const -> const NodeHandle<T>&
        requires std::same_as<T, TextNode>
    {
        if (auto* tn = scene_->texts_.get(id_); tn != nullptr)
        {
            tn->font_size = size;
        }

        return *this;
    }

    // ── Scene method definitions ───────────────────────────────────────────────

    Scene::Scene()
    {
        // Synchronise the transforms pool with the root sentinel already allocated
        // inside the HierarchyPool constructor (index 0, generation 1). Every data
        // pool must have the same number of leading entries so subsequent
        // allocate_at() calls stay in step with the hierarchy indices.
        transforms_.allocate_at(hierarchy_.root().index);
    }

    auto Scene::root() noexcept -> NodeHandle<void>
    {
        return NodeHandle<void>{hierarchy_.root(), this};
    }

    auto Scene::build_draw_list(awen::graphics::DrawList& out) const -> void
    {
        awen::scene::build_draw_list(hierarchy_, transforms_, rects_, circles_, sprites_, texts_, textures_, out);
    }

    auto Scene::load_texture(const std::string& path) -> TextureId
    {
        return textures_.load(path);
    }

    template <typename T>
    auto Scene::add_child_node(NodeId parent, int local_z) -> NodeHandle<T>
    {
        const auto id = hierarchy_.allocate(parent, local_z);

        // Every node, regardless of its visual type, gets a transform slot so
        // that the traversal pass can propagate world positions through void
        // (container/group) nodes as well as typed ones.
        transforms_.allocate_at(id.index);

        if constexpr (std::same_as<T, RectNode>)
        {
            rects_.allocate_at(id.index);
        }
        else if constexpr (std::same_as<T, CircleNode>)
        {
            circles_.allocate_at(id.index);
        }
        else if constexpr (std::same_as<T, SpriteNode>)
        {
            sprites_.allocate_at(id.index);
        }
        else if constexpr (std::same_as<T, TextNode>)
        {
            texts_.allocate_at(id.index);
        }

        return NodeHandle<T>{id, this};
    }
}
